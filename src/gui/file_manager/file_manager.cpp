#include "file_manager/file_manager.h"

#include "core/log.h"

#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"

#include "gui/gui_globals.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>

file_manager::file_manager(QObject* parent) : QObject(parent), m_file_watcher(new QFileSystemWatcher(this)), m_file_open(false)
{
    connect(m_file_watcher, &QFileSystemWatcher::fileChanged, this, &file_manager::handle_file_changed);
    connect(m_file_watcher, &QFileSystemWatcher::directoryChanged, this, &file_manager::handle_directory_changed);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &file_manager::autosave);
}

file_manager* file_manager::get_instance()
{
    static file_manager manager;
    return &manager;
}

void file_manager::handle_program_arguments(const program_arguments& args)
{
    if (args.is_option_set("--input-file"))
    {
        auto file_name = hal::path(args.get_parameter("--input-file"));
        log_info("gui", "GUI started with file {}.", file_name.string());
        open_file(QString::fromStdString(file_name.string()));
    }
}

bool file_manager::file_open() const
{
    return m_file_open;
}

void file_manager::autosave()
{
    log_info("gui", "saving a backup in case something goes wrong...");

    netlist_serializer::serialize_to_file(g_netlist, m_shadow_file_name.toStdString());
}

QString file_manager::file_name() const
{
    if (m_file_open)
        return m_file_name;

    return QString();
}

void file_manager::file_successfully_loaded(QString file_name)
{
    // autosave every 60s
    m_timer->start(60 * 1000);

    m_file_name        = file_name;
    m_shadow_file_name = get_shadow_file(file_name);
    m_file_watcher->addPath(m_file_name);
    m_file_open = true;
    update_recent_files(m_file_name);
    g_python_context = std::make_unique<python_context>();    // HANDLE PYTHON CONTEXT SEPARATELY
    g_graph_context_manager.create_top_context();
    Q_EMIT file_opened(m_file_name);
}

QString file_manager::get_shadow_file(QString file)
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

void file_manager::open_file(QString file_name)
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
            QMessageBox msgBox(QMessageBox::Question, "Resume previous work", "A .hal file exists for the selected netlist.", QMessageBox::Ok | QMessageBox::Close);

            msgBox.setButtonText(QMessageBox::Ok, "Load .hal file");
            msgBox.setButtonText(QMessageBox::Close, "Parse " + extension + " file");

            if (msgBox.exec() == QMessageBox::Ok)
            {
                file_name         = hal_file_name;
                logical_file_name = hal_file_name;
            }
        }
    }

    log_manager& lm    = log_manager::get_instance();
    hal::path log_path = file_name.toStdString();
    lm.set_file_name(hal::path(log_path.replace_extension(".log")));

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
        std::shared_ptr<netlist> netlist = netlist_factory::load_netlist(file_name.toStdString());
        if (netlist)
        {
            g_netlist = netlist;
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

    QString language = "";
    if (file_name.endsWith(".v"))
        language = "verilog";
    else if (file_name.endsWith(".vhd") || file_name.endsWith(".vhdl"))
        language = "vhdl";

    if (language == "")
    {
        std::string error_message("Unable to determine file language. Files have to end with .v, .vhd or .vhdl");
        log_error("gui", "{}", error_message);
        display_error_message(QString::fromStdString(error_message));
        return;
    }

    QList<QPair<std::string, std::shared_ptr<netlist>>> list;

    for (const auto& key : gate_library_manager::get_gate_libraries())
    {
        std::string lib = key.first;

        log_info("gui", "Trying to use gate library '{}'...", lib);
        std::shared_ptr<netlist> netlist = netlist_factory::load_netlist(file_name.toStdString(), language.toStdString(), lib);

        if (netlist)
        {
            list.append(QPair(lib, netlist));
        }
        else
        {
            log_info("gui", "Failed");
        }
    }

    if (list.isEmpty())
    {
        std::string error_message("Unable to find a compatible gate library. Deserialization failed!");
        log_error("gui", "{}", error_message);
        display_error_message(QString::fromStdString(error_message));
        return;
    }

    if (list.length() == 1)
    {
        log_info("gui", "One compatible gate library found.");
        g_netlist = list.at(0).second;
    }

    else
    {
        log_info("gui", "{} compatible gate libraries found. User has to select one.", list.length());
        QInputDialog dialog;

        QStringList libs;

        for (auto& element : list)
            libs.append(QString::fromStdString(element.first));

        dialog.setComboBoxItems(libs);
        dialog.setWindowTitle("Select gate library");
        dialog.setLabelText("The specified file can be processed with more than one gate library. Please select which library should be used:");

        if (dialog.exec())
        {
            std::string selection = dialog.textValue().toStdString();

            for (auto& element : list)
            {
                if (element.first == selection)
                    g_netlist = element.second;
            }
        }
        else
            return;
    }

    file_successfully_loaded(logical_file_name);
}

void file_manager::close_file()
{
    if (!m_file_open)
        return;

    m_timer->stop();

    // CHECK DIRTY AND TRIGGER SAVE ROUTINE

    m_file_watcher->removePath(m_file_name);
    m_file_name = "";
    m_file_open = false;

    if (QFileInfo::exists(m_shadow_file_name) && QFileInfo(m_shadow_file_name).isFile())
    {
        QFile(m_shadow_file_name).remove();
    }

    Q_EMIT file_closed();
}

void file_manager::handle_file_changed(const QString& path)
{
    Q_EMIT file_changed(path);
}

void file_manager::handle_directory_changed(const QString& path)
{
    Q_EMIT file_directory_changed(path);
}

void file_manager::update_recent_files(const QString& file) const
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
    std::vector<hal::path> files;
    for (QString& string : list)
    {
        hal::path file_path(string.toStdString());
        bool skip = false;
        for (const auto& other : files)
        {
            if (hal::fs::equivalent(file_path, other))
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

void file_manager::display_error_message(QString error_message)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText(error_message);
    msgBox.setStyleSheet("QLabel{min-width: 600px;}");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}
