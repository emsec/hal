#include "gui/file_manager/file_manager.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/project_manager.h"
#include "hal_core/utilities/project_directory.h"

#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/settings/settings_items/settings_item_spinbox.h"
#include "gui/file_manager/import_netlist_dialog.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QTextStream>
#include <QApplication>
#include <QDir>

namespace hal
{
    FileManager::FileManager(QObject* parent) : QObject(parent), mFileWatcher(new QFileSystemWatcher(this)), mFileOpen(false)
    {
        mSettingAutosave = new SettingsItemCheckbox(
            "Autosave",
            "advanced/autosave",
            true,
            "eXpert Settings:Autosave",
            "Specifies wheather HAL should autosave."
        );

        mSettingAutosaveInterval = new SettingsItemSpinbox(
            "Autosave Interval",
            "advanced/autosave_interval",
            30,
            "eXpert Settings:Autosave",
            "Sets after how much time in seconds an autosave will occur."
        );

        mAutosaveEnabled = mSettingAutosave->value().toBool();
        mAutosaveInterval = mSettingAutosaveInterval->value().toInt();

        if (mAutosaveInterval < 30)    // failsafe in case somebody sets "0" in the .ini
            mAutosaveInterval = 30;

        connect(mSettingAutosave, &SettingsItemCheckbox::boolChanged, this, [this](bool value){
            mAutosaveEnabled = value;
            if (mTimer->isActive())
                mTimer->start(mAutosaveInterval * 1000);
        });

        connect(mSettingAutosaveInterval, &SettingsItemSpinbox::intChanged, this, [this](int value){
            mAutosaveInterval = value;
            if (mTimer->isActive())
                mTimer->start(mAutosaveInterval * 1000);
        });

        connect(mFileWatcher, &QFileSystemWatcher::fileChanged, this, &FileManager::handleFileChanged);
        connect(mFileWatcher, &QFileSystemWatcher::directoryChanged, this, &FileManager::handleDirectoryChanged);
        mTimer = new QTimer(this);
        connect(mTimer, &QTimer::timeout, this, &FileManager::autosave);
    }

    FileManager* FileManager::get_instance()
    {
        static FileManager manager;
        return &manager;
    }

    void FileManager::handleProgramArguments(const ProgramArguments& args)
    {
        if (args.is_option_set("--input-file"))
        {
            auto fileName = std::filesystem::path(args.get_parameter("--input-file"));
            log_info("gui", "GUI started with file {}.", fileName.string());
            importFile(QString::fromStdString(fileName.string()));
        }
    }

    bool FileManager::fileOpen() const
    {
        return mFileOpen;
    }

    void FileManager::autosave()
    {
        if (!mShadowFileName.isEmpty() && mAutosaveEnabled)
        {
            log_info("gui", "saving a backup in case something goes wrong...");
            netlist_serializer::serialize_to_file(gNetlist, mShadowFileName.toStdString());
        }
    }

    QString FileManager::fileName() const
    {
        if (mFileOpen)
            return mFileName;

        return QString();
    }

    void FileManager::watchFile(const QString& fileName)
    {
        if (fileName == mFileName)
        {
            return;
        }

        if (!mFileName.isEmpty())
        {
            mFileWatcher->removePath(mFileName);
            removeShadowFile();
        }

        mTimer->stop();

        if (!fileName.isEmpty())
        {
            log_info("gui", "watching current file '{}'", fileName.toStdString());

            // autosave periodically
            // (we also start the timer if autosave is disabled since it's way
            // easier to just do nothing when the timer fires instead of messing
            // with complicated start/stop conditions)
            mTimer->start(mAutosaveInterval * 1000);

            mFileName        = fileName;
            mShadowFileName = getShadowFile(fileName);
            mFileWatcher->addPath(mFileName);
            mFileOpen = true;
            updateRecentFiles(mFileName);
        }
    }

    void FileManager::projectSuccessfullyLoaded(QString& projectDir, QString& file)
    {
        watchFile(file);
        Q_EMIT projectOpened(projectDir, file);
        gNetlistRelay->debugHandleFileOpened();
        gContentManager->handleOpenDocument(file);
    }

    void FileManager::fileSuccessfullyLoaded(QString file)
    {
        watchFile(file);
        Q_EMIT fileOpened(file);
    }

    void FileManager::removeShadowFile()
    {
        if (QFileInfo::exists(mShadowFileName) && QFileInfo(mShadowFileName).isFile())
        {
            QFile(mShadowFileName).remove();
        }
    }

    QString FileManager::getShadowFile(QString file)
    {
        QString shadow_file_name;
        if (file.contains('/'))
        {
            shadow_file_name = file.left(file.lastIndexOf('/') + 1) + "~" + file.right(file.size() - file.lastIndexOf('/') - 1);
        }
        else
        {
            shadow_file_name = "~" + file;
        }
        return shadow_file_name.left(shadow_file_name.lastIndexOf('.')) + ".hal";
    }

    void FileManager::importFile(QString filename)
    {
        ImportNetlistDialog ind(filename, qApp->activeWindow());
        if (ind.exec() != QDialog::Accepted) return;
        QString projdir = ind.projectDirectory();
        if (QFileInfo(projdir).exists())
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "Project directory <" + projdir + "> already exists");
            return;
        }
        if (!QDir().mkpath(projdir))
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "Error creating project directory <" + projdir + ">");
            return;
        }

        QDir projectDir(projdir);
        QString netlistFilename = filename;
        if (ind.isMoveNetlistChecked())
        {
            netlistFilename = projectDir.absoluteFilePath(QFileInfo(filename).fileName());
            QDir().rename(filename,netlistFilename);
        }

        ProjectManager* pm = ProjectManager::instance();
        pm->set_project_directory(projdir.toStdString());

        deprecatedOpenFile(netlistFilename);
    }

    void FileManager::openProject(QString projPath)
    {
        ProjectManager* pm = ProjectManager::instance();
        pm->set_project_directory(projPath.toStdString());
        if (!pm->deserialize())
        {
            QString errorMsg = QString("Error opening project <%1>").arg(projPath);
            log_error("gui", "{}", errorMsg.toStdString());
            displayErrorMessage(errorMsg);
            return;
        }

        gNetlistOwner = std::move(pm->get_netlist());
        gNetlist       = gNetlistOwner.get();
        gNetlistRelay->registerNetlistCallbacks();
        QString filename = QString::fromStdString(pm->get_netlist_filename());
        projectSuccessfullyLoaded(projPath, filename);

        LogManager& lm                 = LogManager::get_instance();
        ProjectDirectory pdir = pm->get_project_directory();
        std::filesystem::path log_path = pdir.get_filename(".log");
        lm.set_file_name(log_path);
    }

    void FileManager::deprecatedOpenFile(QString filename)
    {
        QString logical_file_name = filename;

        if (gNetlist)
        {
            // ADD ERROR MESSAGE
            return;
        }

        if (filename.isEmpty())
        {
            QString errorMsg("Unable to open file. File name is empty");
            log_error("gui", "{}", errorMsg.toStdString());
            displayErrorMessage(errorMsg);
            return;
        }

        if (!filename.endsWith(".hal"))
        {
            QString hal_file_name = filename.left(filename.lastIndexOf('.')) + ".hal";
            QString extension     = filename.right(filename.size() - filename.lastIndexOf('.'));

            if (QFileInfo::exists(hal_file_name) && QFileInfo(hal_file_name).isFile())
            {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setWindowTitle("Resume previous work");
                msgBox.setText("A .hal file exists for the selected netlist.");
                auto parse_hal_btn = msgBox.addButton("Load .hal file", QMessageBox::ActionRole);
                auto parse_hdl_btn = msgBox.addButton("Parse " + extension + " file", QMessageBox::ActionRole);
                msgBox.addButton("Abort", QMessageBox::RejectRole);

                QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                QGridLayout* layout           = (QGridLayout*)msgBox.layout();
                layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                msgBox.exec();

                if (msgBox.clickedButton() == (QAbstractButton*)parse_hal_btn)
                {
                    filename         = hal_file_name;
                    logical_file_name = hal_file_name;
                }
                else if (msgBox.clickedButton() != (QAbstractButton*)parse_hdl_btn)
                {
                    return;
                }
            }
        }

        LogManager& lm                 = LogManager::get_instance();
        std::filesystem::path log_path = filename.toStdString();
        lm.set_file_name(std::filesystem::path(log_path.replace_extension(".log")));

        if (filename.endsWith(".hal"))
        {
            QString shadow_file_name = getShadowFile(filename);

            if (QFileInfo::exists(shadow_file_name) && QFileInfo(shadow_file_name).isFile())
            {
                QString message =
                    "It seems that HAL crashed during your last session. Last autosave was on " + QFileInfo(shadow_file_name).lastModified().toString("dd.MM.yyyy hh:mm") + ". Restore that state?";
                if (QMessageBox::question(nullptr, "HAL did not exit cleanly", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                {
                    // logical_file_name is not changed
                    filename = shadow_file_name;
                }
            }
        }

        QFile file(filename);

        if (!file.open(QFile::ReadOnly))
        {
            std::string error_message("Unable to open file" + filename.toStdString());
            log_error("gui", "Unable to open file '{}'", error_message);
            displayErrorMessage(QString::fromStdString(error_message));
            return;
        }

        if (filename.endsWith(".hal"))
        {
            // event_controls::enable_all(false); won't get events until callbacks are registered
            auto netlist = netlist_factory::load_netlist(filename.toStdString());
            // event_controls::enable_all(true);
            if (netlist)
            {
                gNetlistOwner = std::move(netlist);
                gNetlist       = gNetlistOwner.get();
                gNetlistRelay->registerNetlistCallbacks();
                fileSuccessfullyLoaded(logical_file_name);
            }
            else
            {
                std::string error_message("Failed to create netlist from .hal file");
                log_error("gui", "{}", error_message);
                displayErrorMessage(QString::fromStdString(error_message));
            }

            return;
        }

        QString lib_file_name = filename.left(filename.lastIndexOf('.')) + ".lib";
        if (QFileInfo::exists(lib_file_name) && QFileInfo(lib_file_name).isFile())
        {
            log_info("gui", "Trying to use gate library {}.", lib_file_name.toStdString());

            // event_controls::enable_all(false);
            auto netlist = netlist_factory::load_netlist(filename.toStdString(), lib_file_name.toStdString());
            // event_controls::enable_all(true);

            if (netlist)
            {
                gNetlistOwner = std::move(netlist);
                gNetlist       = gNetlistOwner.get();
                gNetlistRelay->registerNetlistCallbacks();
                fileSuccessfullyLoaded(logical_file_name);
                return;
            }
            else
            {
                log_error("gui", "Failed using gate library {}.", lib_file_name.toStdString());
            }
        }

        log_info("gui", "Searching for (other) compatible netlists.");

        // event_controls::enable_all(false);
        std::vector<std::unique_ptr<Netlist>> netlists = netlist_factory::load_netlists(filename.toStdString());
        // event_controls::enable_all(true);

        if (netlists.empty())
        {
            std::string error_message("Unable to find a compatible gate library. Deserialization failed!");
            log_error("gui", "{}", error_message);
            displayErrorMessage(QString::fromStdString(error_message));
            return;
        }
        else if (netlists.size() == 1)
        {
            log_info("gui", "One compatible gate library found.");
            gNetlistOwner = std::move(netlists.at(0));
            gNetlist       = gNetlistOwner.get();
            gNetlistRelay->registerNetlistCallbacks();
        }
        else
        {
            log_info("gui", "{} compatible gate libraries found. User has to select one.", netlists.size());

            QInputDialog dialog;
            QStringList libs;

            for (auto& n : netlists)
            {
                libs.append(QString::fromStdString(n->get_gate_library()->get_name()));
            }

            dialog.setComboBoxItems(libs);
            dialog.setWindowTitle("Select gate library");
            dialog.setLabelText("The specified file can be processed with more than one gate library. Please select which library should be used:");

            if (dialog.exec())
            {
                std::string selection = dialog.textValue().toStdString();

                for (auto& n : netlists)
                {
                    if (n->get_gate_library()->get_name() == selection)
                    {
                        gNetlistOwner = std::move(n);
                        gNetlist       = gNetlistOwner.get();
                        gNetlistRelay->registerNetlistCallbacks();
                    }
                }
            }
            else
            {
                return;
            }
        }

        fileSuccessfullyLoaded(logical_file_name);
    }

    void FileManager::closeFile()
    {
        if (!mFileOpen)
            return;

        mTimer->stop();
        Q_EMIT fileAboutToClose(mFileName);

        // CHECK DIRTY AND TRIGGER SAVE ROUTINE

        gNetlistRelay->unregisterNetlistCallbacks();
        mFileWatcher->removePath(mFileName);
        mFileName = "";
        mFileOpen = false;

        removeShadowFile();

        gNetlistOwner.reset();
        gNetlist = nullptr;

        gNetlistRelay->debugHandleFileClosed();

        Q_EMIT fileClosed();
    }

    void FileManager::handleFileChanged(const QString& path)
    {
        Q_EMIT fileChanged(path);
    }

    void FileManager::handleDirectoryChanged(const QString& path)
    {
        Q_EMIT fileDirectoryChanged(path);
    }

    void FileManager::updateRecentFiles(const QString& file) const
    {
        QStringList list;

        gGuiState->beginReadArray("recent_files");
        for (int i = 0; i < 14; ++i)
        {
            gGuiState->setArrayIndex(i);
            QString state_file = gGuiState->value("file").toString();

            if (state_file.isEmpty())
                continue;

            list.append(gGuiState->value("file").toString());
        }
        gGuiState->endArray();

        int index = list.indexOf(file);
        if (index == -1)
            list.prepend(file);
        else
            list.move(index, 0);

        gGuiState->beginGroup("recent_files");
        gGuiState->remove("");
        gGuiState->endGroup();

        gGuiState->beginWriteArray("recent_files");
        int i = 0;
        std::vector<std::filesystem::path> files;
        for (QString& string : list)
        {
            std::filesystem::path file_path(string.toStdString());
            bool skip = false;
            for (const auto& other : files)
            {
                if (std::filesystem::equivalent(file_path, other))
                {
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                gGuiState->setArrayIndex(i);
                gGuiState->setValue("file", string);
                ++i;
                if (i == 14)
                    break;
            }
        }
        gGuiState->endArray();
    }

    void FileManager::displayErrorMessage(QString error_message)
    {
        QMessageBox msgBox;
        msgBox.setText("Error");
        msgBox.setInformativeText(error_message);
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
}    // namespace hal
