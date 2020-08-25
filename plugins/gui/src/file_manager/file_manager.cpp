#include "gui/file_manager/file_manager.h"

#include "hal_core/utilities/log.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/event_system/event_controls.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"

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

namespace hal
{
    FileManager::FileManager(QObject* parent) : QObject(parent), m_file_watcher(new QFileSystemWatcher(this)), m_file_open(false)
    {
        m_autosave_enabled  = g_settings_manager->get("advanced/autosave").toBool();
        m_autosave_interval = g_settings_manager->get("advanced/autosave_interval").toInt();
        if (m_autosave_interval < 30)    // failsafe in case somebody sets "0" in the .ini
            m_autosave_interval = 30;
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &FileManager::handle_global_setting_changed);

        connect(m_file_watcher, &QFileSystemWatcher::fileChanged, this, &FileManager::handle_file_changed);
        connect(m_file_watcher, &QFileSystemWatcher::directoryChanged, this, &FileManager::handle_directory_changed);
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &FileManager::autosave);
    }

    FileManager* FileManager::get_instance()
    {
        static FileManager manager;
        return &manager;
    }

    void FileManager::handle_program_arguments(const ProgramArguments& args)
    {
        if (args.is_option_set("--input-file"))
        {
            auto file_name = std::filesystem::path(args.get_parameter("--input-file"));
            log_info("gui", "GUI started with file {}.", file_name.string());
            open_file(QString::fromStdString(file_name.string()));
        }
    }

    bool FileManager::file_open() const
    {
        return m_file_open;
    }

    void FileManager::autosave()
    {
        if (!m_shadow_file_name.isEmpty() && m_autosave_enabled)
        {
            log_info("gui", "saving a backup in case something goes wrong...");
            netlist_serializer::serialize_to_file(g_netlist, m_shadow_file_name.toStdString());
        }
    }

    QString FileManager::file_name() const
    {
        if (m_file_open)
            return m_file_name;

        return QString();
    }

    void FileManager::watch_file(const QString& file_name)
    {
        if (file_name == m_file_name)
        {
            return;
        }

        if (!m_file_name.isEmpty())
        {
            m_file_watcher->removePath(m_file_name);
            remove_shadow_file();
        }

        m_timer->stop();

        if (!file_name.isEmpty())
        {
            log_info("gui", "watching current file '{}'", file_name.toStdString());

            // autosave periodically
            // (we also start the timer if autosave is disabled since it's way
            // easier to just do nothing when the timer fires instead of messing
            // with complicated start/stop conditions)
            m_timer->start(m_autosave_interval * 1000);

            m_file_name        = file_name;
            m_shadow_file_name = get_shadow_file(file_name);
            m_file_watcher->addPath(m_file_name);
            m_file_open = true;
            update_recent_files(m_file_name);
        }
    }

    void FileManager::file_successfully_loaded(QString file_name)
    {
        watch_file(file_name);
        Q_EMIT file_opened(m_file_name);
    }

    void FileManager::remove_shadow_file()
    {
        if (QFileInfo::exists(m_shadow_file_name) && QFileInfo(m_shadow_file_name).isFile())
        {
            QFile(m_shadow_file_name).remove();
        }
    }

    QString FileManager::get_shadow_file(QString file)
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

    void FileManager::open_file(QString file_name)
    {
        QString logical_file_name = file_name;

        if (g_netlist)
        {
            // ADD ERROR MESSAGE
            return;
        }

        if (file_name.isEmpty())
        {
            std::string error_message("Unable to open file. File name is empty");
            log_error("gui", "{}", error_message);
            display_error_message(QString::fromStdString(error_message));
            return;
        }

        if (!file_name.endsWith(".hal"))
        {
            QString hal_file_name = file_name.left(file_name.lastIndexOf('.')) + ".hal";
            QString extension     = file_name.right(file_name.size() - file_name.lastIndexOf('.'));

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
                    file_name         = hal_file_name;
                    logical_file_name = hal_file_name;
                }
                else if (msgBox.clickedButton() != (QAbstractButton*)parse_hdl_btn)
                {
                    return;
                }
            }
        }

        LogManager& lm                 = LogManager::get_instance();
        std::filesystem::path log_path = file_name.toStdString();
        lm.set_file_name(std::filesystem::path(log_path.replace_extension(".log")));

        if (file_name.endsWith(".hal"))
        {
            QString shadow_file_name = get_shadow_file(file_name);

            if (QFileInfo::exists(shadow_file_name) && QFileInfo(shadow_file_name).isFile())
            {
                QString message =
                    "It seems that HAL crashed during your last session. Last autosave was on " + QFileInfo(shadow_file_name).lastModified().toString("dd.MM.yyyy hh:mm") + ". Restore that state?";
                if (QMessageBox::question(nullptr, "HAL did not exit cleanly", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                {
                    // logical_file_name is not changed
                    file_name = shadow_file_name;
                }
            }
        }

        QFile file(file_name);

        if (!file.open(QFile::ReadOnly))
        {
            std::string error_message("Unable to open file" + file_name.toStdString());
            log_error("gui", "Unable to open file '{}'", error_message);
            display_error_message(QString::fromStdString(error_message));
            return;
        }

        if (file_name.endsWith(".hal"))
        {
            event_controls::enable_all(false);
            auto netlist = netlist_factory::load_netlist(file_name.toStdString());
            event_controls::enable_all(true);
            if (netlist)
            {
                g_netlist_owner = std::move(netlist);
                g_netlist       = g_netlist_owner.get();
                file_successfully_loaded(logical_file_name);
            }
            else
            {
                std::string error_message("Failed to create netlist from .hal file");
                log_error("gui", "{}", error_message);
                display_error_message(QString::fromStdString(error_message));
            }

            return;
        }

        event_controls::enable_all(false);
        std::vector<std::unique_ptr<Netlist>> netlists = netlist_factory::load_netlists(file_name.toStdString());
        event_controls::enable_all(true);

        if (netlists.empty())
        {
            std::string error_message("Unable to find a compatible gate library. Deserialization failed!");
            log_error("gui", "{}", error_message);
            display_error_message(QString::fromStdString(error_message));
            return;
        }
        else if (netlists.size() == 1)
        {
            log_info("gui", "One compatible gate library found.");
            g_netlist_owner = std::move(netlists.at(0));
            g_netlist       = g_netlist_owner.get();
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
                        g_netlist_owner = std::move(n);
                        g_netlist       = g_netlist_owner.get();
                    }
                }
            }
            else
            {
                return;
            }
        }

        file_successfully_loaded(logical_file_name);
    }

    void FileManager::close_file()
    {
        if (!m_file_open)
            return;

        m_timer->stop();

        // CHECK DIRTY AND TRIGGER SAVE ROUTINE

        m_file_watcher->removePath(m_file_name);
        m_file_name = "";
        m_file_open = false;

        remove_shadow_file();

        Q_EMIT file_closed();
    }

    void FileManager::handle_file_changed(const QString& path)
    {
        Q_EMIT file_changed(path);
    }

    void FileManager::handle_directory_changed(const QString& path)
    {
        Q_EMIT file_directory_changed(path);
    }

    void FileManager::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender);
        if (key == "advanced/autosave")
        {
            m_autosave_enabled = value.toBool();
            if (m_timer->isActive())
            {
                // restart timer so the interval starts NOW
                m_timer->start(m_autosave_interval * 1000);
            }
        }
        else if (key == "advanced/autosave_interval")
        {
            m_autosave_interval = value.toInt();
            if (m_timer->isActive())
            {
                // restart timer with new interval
                m_timer->start(m_autosave_interval * 1000);
            }
        }
    }

    void FileManager::update_recent_files(const QString& file) const
    {
        QStringList list;

        g_gui_state.beginReadArray("recent_files");
        for (int i = 0; i < 14; ++i)
        {
            g_gui_state.setArrayIndex(i);
            QString state_file = g_gui_state.value("file").toString();

            if (state_file.isEmpty())
                continue;

            list.append(g_gui_state.value("file").toString());
        }
        g_gui_state.endArray();

        int index = list.indexOf(file);
        if (index == -1)
            list.prepend(file);
        else
            list.move(index, 0);

        g_gui_state.beginGroup("recent_files");
        g_gui_state.remove("");
        g_gui_state.endGroup();

        g_gui_state.beginWriteArray("recent_files");
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
                g_gui_state.setArrayIndex(i);
                g_gui_state.setValue("file", string);
                ++i;
                if (i == 14)
                    break;
            }
        }
        g_gui_state.endArray();
    }

    void FileManager::display_error_message(QString error_message)
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
