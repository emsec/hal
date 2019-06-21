#include "file_manager/file_manager.h"

#include "core/log.h"

#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"

#include "gui_globals.h"

#include <QFile>
#include <QFileSystemWatcher>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>

file_manager::file_manager(QObject* parent) : QObject(parent), m_file_watcher(new QFileSystemWatcher(this)), m_file_open(false)
{
    connect(m_file_watcher, &QFileSystemWatcher::fileChanged, this, &file_manager::handle_file_changed);
    connect(m_file_watcher, &QFileSystemWatcher::directoryChanged, this, &file_manager::handle_directory_changed);
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

bool file_manager::is_document_open()
{
    return m_file_open;
}

QString file_manager::file_name() const
{
    if (m_file_open)
        return m_file_name;

    return QString();
}

void file_manager::open_file(const QString& file_name)
{
    if (g_netlist)
    {
        // TODO HANDLE THIS CASE
        return;
    }

    if (file_name.isEmpty())
    {
        std::string error_message("Unable to open file. File name is empty");
        log_error("gui", "{}", error_message);
        display_error_message(QString::fromStdString(error_message));
        return;
    }

    QFile file(file_name);

    if (!file.open(QFile::ReadOnly))
    {
        std::string error_message("Unable to open file" + file_name.toStdString());
        log_error("gui", "Unable to open file '{}'", error_message);
        display_error_message(QString::fromStdString(error_message));
        return;
    }

    log_manager& lm    = log_manager::get_instance();
    hal::path log_path = file_name.toStdString();
    lm.set_file_name(hal::path(log_path.replace_extension(".log")));

    m_file_name = file_name;

    if (file_name.endsWith(".hal"))
    {
        std::shared_ptr<netlist> netlist = netlist_factory::load_netlist(file_name.toStdString());
        if (netlist)
        {
            g_netlist = netlist;
            m_file_watcher->addPath(m_file_name);
            m_file_open = true;
            update_recent_files(m_file_name);
            g_python_context = std::make_unique<python_context>();    // TODO HANDLE PYTHON CONTEXT SEPARATELY
            Q_EMIT file_opened(m_file_name);
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

    /* ORIGINAL CODE
 *
    std::vector<std::string> gate_library_names;

    // guessing the best library
    {
        // collect all names
        for (const auto& key : gate_library_manager::get_gate_libraries())
        {
            gate_library_names.push_back(key.first);
        }

        // load the first 100 lines of the file
        std::string preview = "";
        QTextStream in(&file);
        for (int i = 0; (i < 100) && (!in.atEnd()); ++i)
        {
            QString line = in.readLine();
            preview += core_utils::to_lower(line.toStdString());
        }

        // compute a 'likelihood score' for each library
        std::map<int, std::set<std::string>> gate_library_guesses;
        for (const auto& lib : gate_library_names)
        {
            // split the library name into tags by '_'
            auto tags = core_utils::split(core_utils::to_lower(lib), '_');
            // the full name is also a tag
            tags.insert(tags.begin(), core_utils::to_lower(lib));

            // now count how many of the tags are in the preview
            int score = 0;
            for (const auto& tag : tags)
            {
                if (tag.size() > 2 && preview.find(tag) != std::string::npos)
                {
                    score++;
                }
            }
            gate_library_guesses[score].insert(lib);
        }

        // sort the names by their score (map autosorts ascending)
        gate_library_names.clear();
        for (const auto& pair : gate_library_guesses)
        {
            for (const auto& lib : pair.second)
            {
                gate_library_names.insert(gate_library_names.begin(), lib);
            }
        }
    }

    // gate_library_names now contains all libraries, with the most likely one in first place
    for (const auto& lib : gate_library_names)
    {
        log_info("gui", "Trying to use gate library '{}'...", lib);
        std::shared_ptr<netlist> n = netlist_factory::load_netlist(file_name.toStdString(), language.toStdString(), lib);
        if (n == nullptr)
        {
            continue;
        }

        g_netlist = n;
        m_file_watcher->addPath(m_file_name);
        m_file_open = true;
        update_recent_files(file_name);
        g_python_context = std::make_unique<python_context>();
        Q_EMIT file_opened(file_name);
        return;
    }
    log_error("gui", "Unable to find a compatible gate library. Deserialization failed!");

END OF ORIGINAL CODE */

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
        log_error("gui", "{}" , error_message);
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

    m_file_watcher->addPath(m_file_name);
    m_file_open = true;
    update_recent_files(file_name);
    g_python_context = std::make_unique<python_context>();
    Q_EMIT file_opened(file_name);
}

void file_manager::close_file()
{
    if (!m_file_open)
        return;

    // TODO CHECK DIRTY AND TRIGGER SAVE ROUTINE

    m_file_watcher->removePath(m_file_name);
    m_file_name = "";
    m_file_open = false;
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

void file_manager::update_recent_files(const QString& file)
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
