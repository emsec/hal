#include "gui/plugin_access_manager/extended_cli_dialog.h"
#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_interface_gui.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include <QChar>
#include <iostream>

namespace hal
{
    ExtendedCliDialog::ExtendedCliDialog(QString plugin_name, QWidget* parent) : QDialog(parent)
    {
        mContentLayout = new QVBoxLayout(this);
        mFormLayout    = new QFormLayout();
        mStatusMessage = new QLabel(this);
        mButtonBox     = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        setWindowTitle(plugin_name + " arguments");
        setSizeGripEnabled(true);
        setMinimumWidth(600);
        setLayout(mContentLayout);
        mContentLayout->addLayout(mFormLayout);

        mStatusMessage->setStyleSheet("QLabel { background-color: rgba(64, 64, 64, 1);color: rgba(255, 0, 0, 1);border: 1px solid rgba(255, 0, 0, 1)}");
        mStatusMessage->setText("Argument list invalid");
        mStatusMessage->setAlignment(Qt::AlignCenter);
        mStatusMessage->setMinimumHeight(90);
        mStatusMessage->hide();
        mContentLayout->addWidget(mStatusMessage);
        mContentLayout->addWidget(mButtonBox, Qt::AlignBottom);

        connect(mButtonBox, SIGNAL(accepted()), this, SLOT(parseArguments()));
        connect(mButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

        mArgv = nullptr;
        setup(plugin_name.toStdString());
    }

    ProgramArguments ExtendedCliDialog::getArgs()
    {
        return mArgs;
    }

    void ExtendedCliDialog::parseArguments()
    {
        std::vector<char*> temp_vector;

        std::string string = "hal";
        char* cstring      = new char[string.size() + 1];
        std::copy(string.begin(), string.end(), cstring);
        cstring[string.size()] = '\0';
        temp_vector.push_back(cstring);

        for (auto pair : mVector)
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

        auto options = mPlugin->get_cli_options();
        mArgs       = options.parse(argc, const_cast<const char**>(argv));
        // ProgramOptions Stuff
        for (int i = 0; i < argc; ++i)
            delete[] argv[i];

        delete[] argv;
        mStatusMessage->hide();
        accept();
    }

    void ExtendedCliDialog::setup(std::string plugin_name)
    {
        mPlugin = plugin_manager::get_plugin_instance<CLIPluginInterface>(plugin_name, false);
        if (mPlugin == nullptr)
        {
            return;
        }

        for (auto option_tupel : mPlugin->get_cli_options().get_options())
        {
            QString mFlag        = QString::fromStdString(*std::get<0>(option_tupel).begin());
            QString mDescription = QString::fromStdString(std::get<1>(option_tupel));
            QPushButton* button = new QPushButton(mFlag, this);
            button->setStyleSheet("QPushButton:mChecked { background-color: rgba(114, 114, 0, 1); }");
            QLineEdit* LineEdit = new QLineEdit(this);
            button->setToolTip(mDescription);
            button->setCheckable(true);
            mVector.push_back(std::make_pair(button, LineEdit));
            mFormLayout->addRow(button, LineEdit);
        }
    }
}
