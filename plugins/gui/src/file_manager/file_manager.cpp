#include "gui/file_manager/file_manager.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/project_directory.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/settings/settings_items/settings_item_spinbox.h"
#include "gui/file_manager/import_netlist_dialog.h"
#include "gui/file_manager/new_project_dialog.h"

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
#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

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

    bool FileManager::fileOpen() const
    {
        return mFileOpen;
    }

    void FileManager::autosave()
    {
        ProjectManager* pm = ProjectManager::instance();
        if (pm->get_project_status() != ProjectManager::ProjectStatus::NONE && mAutosaveEnabled)
        {
            if (gPythonContext->pythonThread())
            {
                log_info("gui", "Autosave deferred while python script is running...");
                return;
            }
            log_info("gui", "saving a backup in case something goes wrong...");
            if (!ProjectManager::instance()->serialize_project(gNetlist, true))
                log_warning("gui", "Autosave failed to create project backup to directory '{}'.", pm->get_project_directory().get_shadow_dir().string());
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
            removeShadowDirectory();
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
            mFileWatcher->addPath(mFileName);
            mFileOpen = true;
            updateRecentFiles(mFileName);
        }
    }

    void FileManager::emitProjectSaved(QString& projectDir, QString& file)
    {
        Q_EMIT projectSaved(projectDir, file);
    }

    void FileManager::projectSuccessfullyLoaded(QString& projectDir, QString& file)
    {
        watchFile(file);
        Q_EMIT projectOpened(projectDir, file);
        gNetlistRelay->debugHandleFileOpened();
        gContentManager->handleOpenDocument(file);
        ProjectManager::instance()->set_project_status(ProjectManager::ProjectStatus::OPENED);
    }


    FileManager::DirectoryStatus FileManager::directoryStatus(const QString& pathname)
    {
        QFileInfo info(pathname);
        if (!info.exists()) return NotExisting;

        if (info.isFile())
        {
            if (netlist_parser_manager::can_parse(pathname.toStdString()))
                return IsFile;
            else
                return InvalidExtension;
        }

        if (info.isDir())
        {
            if (!info.suffix().isEmpty()) return InvalidExtension;
            QFile ff(QDir(pathname).absoluteFilePath(QString::fromStdString(ProjectManager::s_project_file)));
            if (ff.exists())
            {
                if (!ff.open(QIODevice::ReadOnly))
                    return ParseError;
                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(ff.readAll(),&err);
                if (err.error != QJsonParseError::NoError)
                    return ParseError;
                if (!doc.isObject())
                    return ParseError;
                if (!doc.object().contains("netlist"))
                    return NetlistError;
                QString nl = doc.object()["netlist"].toString();
                if (nl.isEmpty()) return NetlistError;
                if (QFileInfo(nl).isAbsolute())
                {
                    if (!QFileInfo(nl).exists())
                        return NetlistError;
                }
                else
                {
                    if (!QFileInfo(QDir(pathname).absoluteFilePath(nl)).exists())
                        return NetlistError;
                }
                QString gl = doc.object()["gate_library"].toString();
                if (gl.isEmpty()) return GatelibError;
                if (QFileInfo(gl).isAbsolute())
                {
                    if (!QFileInfo(gl).exists())
                        return GatelibError;
                }
                else
                {
                    if (!QFileInfo(QDir(pathname).absoluteFilePath(gl)).exists())
                        return GatelibError;
                }
                return ProjectDirectory;
            }
            else
                return OtherDirectory;
        }

        return UnknownDirectoryEntry;
    }

    QString FileManager::directoryStatusText(DirectoryStatus stat)
    {
        switch (stat)
        {
        case ProjectDirectory:
            return QString("seems to be a HAL project");
        case OtherDirectory:
            return QString("no HAL project file found in current directory");
        case IsFile:
            return QString("entry is not a directory but a file");
        case NotExisting:
            return QString("directory does not exist");
        case InvalidExtension:
            return QString("HAL project directory must not have an extension");
        case ParseError:
            return QString("HAL project file parse error");
        case NetlistError:
            return QString("cannot find HAL netlist");
        case GatelibError:
            return QString("cannot find HAL gate library");
        case UnknownDirectoryEntry:
            return QString("entry is neither directory nor file");
        }

        return QString();
    }

    void FileManager::fileSuccessfullyLoaded(QString file)
    {
        watchFile(file);
        Q_EMIT fileOpened(file);
        ProjectManager::instance()->set_project_status(ProjectManager::ProjectStatus::OPENED);
    }

    void FileManager::removeShadowDirectory()
    {
        ProjectManager* pm = ProjectManager::instance();
        if (pm->get_project_status() == ProjectManager::ProjectStatus::NONE) return;
        QDir shDir(QString::fromStdString(pm->get_project_directory().get_shadow_dir()));
        if (shDir.exists())
        {
            shDir.removeRecursively();
            QDir().mkpath(shDir.absolutePath());
        }

    }

    void FileManager::newProject()
    {
        NewProjectDialog npr(qApp->activeWindow());
        if (npr.exec() != QDialog::Accepted) return;

        QString projdir = npr.projectDirectory();
        if (projdir.isEmpty()) return;

        QString gatelib = npr.gateLibraryPath();
        if (gatelib.isEmpty())
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "Cannot create project <" + projdir + ">, no gate library selected");
            return;
        }
        GateLibrary* glib = gate_library_manager::load(gatelib.toStdString());
        if (!glib)
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "Cannot create project <" + projdir + ">, cannot load gate library <" + gatelib + ">");
            return;
        }

        ProjectManager* pm = ProjectManager::instance();
        if (!pm->create_project_directory(projdir.toStdString()))
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "Error creating project directory <" + projdir + ">");
            return;
        }

        QDir projectDir(projdir);
        QString netlistFilename = QString::fromStdString(pm->get_netlist_filename());
        if (npr.isCopyGatelibChecked())
        {
            QFileInfo glInfo(gatelib);
            QString targetGateLib = projectDir.absoluteFilePath(glInfo.fileName());
            if (QFile::copy(gatelib,targetGateLib))
                gatelib = targetGateLib;
        }

        std::filesystem::path lpath = pm->get_project_directory().get_default_filename(".log");
        LogManager::get_instance()->set_file_name(lpath);

        gNetlistOwner = netlist_factory::create_netlist(glib);
        gNetlist = gNetlistOwner.get();

        if (!gNetlist)
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "Failed to create <" + projdir + "> with gate library <" + gatelib + ">");
            return;
        }
        gNetlistRelay->registerNetlistCallbacks();
        gFileStatusManager->netlistChanged();
        if (pm->serialize_project(gNetlist))
            gFileStatusManager->netlistSaved();
        projectSuccessfullyLoaded(projdir,netlistFilename);
        log_info("gui", "Created empty project '{}' with gate library '{}'.", projdir.toStdString(), gatelib.toStdString());
    }

    void FileManager::importFile(QString filename)
    {
        // check whether there is already a project with the same name as file-to-be-imported
        QString testProjectExists(filename);
        testProjectExists.remove(QRegularExpression("\\.\\w*$"));
        if (directoryStatus(testProjectExists) == FileManager::ProjectDirectory)
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setWindowTitle("Resume previous work");
            msgBox.setText("A hal project exists for the selected netlist.");
            auto butLoadProject   = msgBox.addButton("Load hal project", QMessageBox::ActionRole);
            auto butImportNetlist = msgBox.addButton("Parse " + QFileInfo(filename).suffix() + " file", QMessageBox::ActionRole);
            msgBox.addButton("Abort", QMessageBox::RejectRole);

            QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
            QGridLayout* layout           = (QGridLayout*)msgBox.layout();
            layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

            msgBox.exec();

            if (msgBox.clickedButton() == (QAbstractButton*)butLoadProject)
            {
                openProject(testProjectExists);
                return;
            }
            else if (msgBox.clickedButton() != (QAbstractButton*)butImportNetlist)
            {
                return;
            }
        }

        ImportNetlistDialog ind(filename, qApp->activeWindow());
        if (ind.exec() != QDialog::Accepted) return;
        QString projdir = ind.projectDirectory();
        if (projdir.isEmpty()) return;

        if (!QFileInfo(projdir).suffix().isEmpty())
        {
            QMessageBox::warning(qApp->activeWindow(),"Aborted", "selected project directory name must not have suffix ." + QFileInfo(projdir).suffix());
            return;
        }


        ProjectManager* pm = ProjectManager::instance();
        if (!pm->create_project_directory(projdir.toStdString()))
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

        QString gatelib = ind.gateLibraryPath();
        if (ind.isCopyGatelibChecked() && !gatelib.isEmpty())
        {
            QFileInfo glInfo(gatelib);
            QString targetGateLib = projectDir.absoluteFilePath(glInfo.fileName());
            if (QFile::copy(gatelib,targetGateLib))
                gatelib = targetGateLib;
        }

        std::filesystem::path lpath = pm->get_project_directory().get_default_filename(".log");
        LogManager::get_instance()->set_file_name(lpath);

        if (deprecatedOpenFile(netlistFilename, gatelib))
        {
            gFileStatusManager->netlistChanged();
            if (gNetlist)
                if (pm->serialize_project(gNetlist))
                    gFileStatusManager->netlistSaved();
            Q_EMIT projectOpened(projectDir.absolutePath(),QString::fromStdString(pm->get_netlist_filename()));
        }
        else
        {
            // failed to create project: if netlist was moved move back before deleting directory
            if (ind.isMoveNetlistChecked())
                QDir().rename(netlistFilename,filename);
            if (pm->remove_project_directory())
                log_info("gui", "Project directory removed since import failed.");
            else
                log_warning("gui", "Failed to remove project directory after failed import attempt");
        }
    }

    void FileManager::moveShadowToProject(const QDir& shDir) const
    {
        QString replaceToken = "/" + QString::fromStdString(ProjectDirectory::s_shadow_dir) + "/";
        for (QFileInfo finfo : shDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot))
        {
            QString targetPath(finfo.absoluteFilePath());
            targetPath.replace(replaceToken,"/");

            if (finfo.isDir())
            {
                QDir().mkpath(targetPath);
                moveShadowToProject(QDir(finfo.absoluteFilePath()));
            }
            else if (finfo.isFile())
            {
                if (QFileInfo(targetPath).exists()) QFile::remove(targetPath);
                QFile::copy(finfo.absoluteFilePath(),targetPath);
                log_info("gui", "File restored from autosave: {}.", targetPath.toStdString());
            }
            else
            {
                log_info("gui","Cannot move {} from autosave to project directory.", finfo.absoluteFilePath().toStdString());
            }
        }
    }

    void FileManager::openProject(QString projPath)
    {
        ProjectManager* pm = ProjectManager::instance();
        pm->set_project_directory(projPath.toStdString());

        QDir shDir(QString::fromStdString(pm->get_project_directory().get_shadow_dir()));
        QFileInfo shInfo(shDir.absoluteFilePath(QString::fromStdString(ProjectManager::s_project_file)));
        if (shInfo.exists() && shInfo.isFile())
        {
            QString message =
                "It seems that HAL crashed during your last session. Last autosave was on " +
                    shInfo.lastModified().toString("dd.MM.yyyy hh:mm") + ". Restore that state?";
            if (QMessageBox::question(nullptr, "HAL did not exit cleanly", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            {
                moveShadowToProject(shDir);
                pm->restore_project_file_from_autosave();
            }
        }

        if (!pm->open_project())
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

        std::filesystem::path lpath = pm->get_project_directory().get_default_filename(".log");
        LogManager::get_instance()->set_file_name(lpath);
    }

    bool FileManager::deprecatedOpenFile(QString filename, QString gatelibraryPath)
    {
        QString logical_file_name = filename;

        if (gNetlist)
        {
            // ADD ERROR MESSAGE
            return false;
        }

        if (filename.isEmpty())
        {
            QString errorMsg("Unable to open file. File name is empty");
            log_error("gui", "{}", errorMsg.toStdString());
            displayErrorMessage(errorMsg);
            return false;
        }

        QFile file(filename);

        if (!file.open(QFile::ReadOnly))
        {
            std::string error_message("Unable to open file" + filename.toStdString());
            log_error("gui", "Unable to open file '{}'", error_message);
            displayErrorMessage(QString::fromStdString(error_message));
            return false;
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
                return true;
            }
            else
            {
                std::string error_message("Failed to create netlist from .hal file");
                log_error("gui", "{}", error_message);
                displayErrorMessage(QString::fromStdString(error_message));
                return false;
            }
        }

        if (!gatelibraryPath.isEmpty())
        {
            log_info("gui", "Trying to use gate library {}.", gatelibraryPath.toStdString());
            auto netlist = netlist_factory::load_netlist(filename.toStdString(), gatelibraryPath.toStdString());

            if (netlist)
            {
                gNetlistOwner = std::move(netlist);
                gNetlist       = gNetlistOwner.get();
                gNetlistRelay->registerNetlistCallbacks();
                fileSuccessfullyLoaded(logical_file_name);
                return true;
            }
            else
            {
                log_error("gui", "Failed using gate library {}.", gatelibraryPath.toStdString());
                displayErrorMessage("Failed to open netlist\nwith user selected gate library\n"+gatelibraryPath);
                return false;
            }
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
                return true;
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
            return false;
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
                return false;
            }
        }

        fileSuccessfullyLoaded(logical_file_name);
        return true;
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

        removeShadowDirectory();

        gNetlistOwner.reset();
        gNetlist = nullptr;

        gNetlistRelay->debugHandleFileClosed();
        ProjectManager::instance()->set_project_status(ProjectManager::ProjectStatus::NONE);

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
