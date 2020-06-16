#include "hal_plugin_access_manager/hal_extended_cli_dialog.h"
#include "core/plugin_interface_cli.h"
#include "core/plugin_interface_gui.h"
#include "core/plugin_manager.h"
#include <QChar>
#include <iostream>

hal_extended_cli_dialog::hal_extended_cli_dialog(QString plugin_name, QWidget* parent) : QDialog(parent)
{
    m_content_layout = new QVBoxLayout(this);
    m_form_layout    = new QFormLayout();
    m_status_message = new QLabel(this);
    m_button_box     = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    setWindowTitle(plugin_name + " arguments");
    setSizeGripEnabled(true);
    setMinimumWidth(600);
    setLayout(m_content_layout);
    m_content_layout->addLayout(m_form_layout);

    m_status_message->setStyleSheet("QLabel { background-color: rgba(64, 64, 64, 1);color: rgba(255, 0, 0, 1);border: 1px solid rgba(255, 0, 0, 1)}");
    m_status_message->setText("Argument list invalid");
    m_status_message->setAlignment(Qt::AlignCenter);
    m_status_message->setMinimumHeight(90);
    m_status_message->hide();
    m_content_layout->addWidget(m_status_message);
    m_content_layout->addWidget(m_button_box, Qt::AlignBottom);

    connect(m_button_box, SIGNAL(accepted()), this, SLOT(parse_arguments()));
    connect(m_button_box, SIGNAL(rejected()), this, SLOT(reject()));

    m_argv = nullptr;
    setup(plugin_name.toStdString());
}

ProgramArguments hal_extended_cli_dialog::get_args()
{
    return m_args;
}

void hal_extended_cli_dialog::parse_arguments()
{
    std::vector<char*> temp_vector;

    std::string string = "hal";
    char* cstring      = new char[string.size() + 1];
    std::copy(string.begin(), string.end(), cstring);
    cstring[string.size()] = '\0';
    temp_vector.push_back(cstring);

    for (auto pair : m_vector)
    {
        if (std::get<0>(pair)->isChecked())
        {
            QString qstring = std::get<0>(pair)->text();
            qstring.prepend("--");
            std::string string_shadow = qstring.toStdString();
            char* cstring_shadow      = new char[string_shadow.size() + 1];
            std::copy(string_shadow.begin(), string_shadow.end(), cstring_shadow);
            cstring_shadow[string_shadow.size()] = '\0';
            temp_vector.push_back(cstring_shadow);

            if (!std::get<1>(pair)->text().isEmpty())
            {
                QString qstring_shadow        = std::get<1>(pair)->text();
                std::string std_string_shadow = qstring_shadow.toStdString();
                char* char_cstring_shadow     = new char[std_string_shadow.size() + 1];
                std::copy(std_string_shadow.begin(), std_string_shadow.end(), char_cstring_shadow);
                char_cstring_shadow[std_string_shadow.size()] = '\0';
                temp_vector.push_back(char_cstring_shadow);
            }
        }
    }
    int argc    = temp_vector.size();
    char** argv = new char*[argc + 1];
    for (int i = 0; i < argc; i++)
    {
        argv[i] = temp_vector[i];
    }
    argv[argc] = nullptr;

    //    int i;
    //    printf("argc: %d\n", argc);
    //    for(i=0; i < argc; i++) {
    //        printf("argv[%d]: %s\n", i, argv[i]);
    //    }

    auto options = m_plugin->get_cli_options();
    m_args       = options.parse(argc, const_cast<const char**>(argv));
    //    try
    //    {
    //        .
    //        boost::ProgramOptions::store(boost::ProgramOptions::parse_command_line(argc, argv, (std::get<1>(m_plugin->get_cli_options()))), *m_map);
    //        boost::ProgramOptions::notify(*m_map);
    //
    //        //        ProgramOptions plugin_options;
    //        //        plugin_options.add(*(std::get<1>(m_plugin->get_cli_options())));
    //        //        boost::ProgramOptions::store(boost::ProgramOptions::command_line_parser(argc, argv).options(plugin_options).run(), *m_map);
    //        //        boost::ProgramOptions::notify(*m_map);
    //    }
    //    catch (const std::exception& e)
    //    {
    //        std::cout << e.what() << std::endl;
    //        for (int i = 0; i < argc; ++i)
    //            delete[] argv[i];
    //
    //        delete[] argv;
    //        delete m_map;
    //        m_map = nullptr;
    //        return;
    //    }
    for (int i = 0; i < argc; ++i)
        delete[] argv[i];

    delete[] argv;
    m_status_message->hide();
    accept();
}

void hal_extended_cli_dialog::setup(std::string plugin_name)
{
    m_plugin = PluginManager::get_plugin_instance<CLIPluginInterface>(plugin_name, false);
    if (m_plugin == nullptr)
    {
        return;
    }

    for (auto option_tupel : m_plugin->get_cli_options().get_options())
    {
        QString flag        = QString::fromStdString(*std::get<0>(option_tupel).begin());
        QString description = QString::fromStdString(std::get<1>(option_tupel));
        QPushButton* button = new QPushButton(flag, this);
        button->setStyleSheet("QPushButton:checked { background-color: rgba(114, 114, 0, 1); }");
        QLineEdit* line_edit = new QLineEdit(this);
        button->setToolTip(description);
        button->setCheckable(true);
        m_vector.push_back(std::make_pair(button, line_edit));
        m_form_layout->addRow(button, line_edit);
    }
}
