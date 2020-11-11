#include "gui/plugin_management/plugin_arguments_widget.h"

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/program_arguments.h"
#include "gui/plugin_management/plugin_schedule_manager.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

namespace hal
{
    PluginArgumentsWidget::PluginArgumentsWidget(QWidget* parent) : QFrame(parent), mFormLayout(new QFormLayout())
    {
        setLayout(mFormLayout);
    }

    ProgramArguments PluginArgumentsWidget::getArgs()
    {
        std::vector<char*> temp_vector;

        QString string = "hal";
        temp_vector.push_back(toHeapCstring(string));

        for (QPair<QPushButton*, QLineEdit*> pair : mVector)
        {
            if (pair.first->isChecked())
            {
                string = pair.first->text();
                string.prepend("--");
                temp_vector.push_back(toHeapCstring(string));

                if (!pair.second->text().isEmpty())
                {
                    string = pair.second->text();
                    temp_vector.push_back(toHeapCstring(string));
                }
            }
        }

        int argc    = temp_vector.size();
        char** argv = new char*[argc + 1];

        for (int i = 0; i < argc; i++)
            argv[i] = temp_vector[i];

        argv[argc] = nullptr;

        //    int i;
        //    printf("argc: %d\n", argc);
        //    for(i=0; i < argc; i++) {
        //        printf("argv[%d]: %s\n", i, argv[i]);
        //    }

        ProgramOptions options     = mPlugin->get_cli_options();
        ProgramArguments arguments = options.parse(argc, const_cast<const char**>(argv));

        for (int i = 0; i < argc; ++i)
            delete[] argv[i];

        delete[] argv;

        return arguments;
    }

    //void PluginArgumentsWidget::setupPluginLayout(const QString& plugin)
    //{
    //    for (QPair<QPushButton*, QLineEdit*>& pair : mVector)
    //    {
    //        pair.first->hide();
    //        pair.second->hide();
    //        mFormLayout->removeWidget(pair.first);
    //        mFormLayout->removeWidget(pair.second);
    //        pair.first->deleteLater();
    //        pair.second->deleteLater();
    //    }
    //    mVector.clear();

    //    //USE EXECUTION MANAGER

    //    i_factory* factory_ptr = plugin_manager::get_plugin_factory(plugin.toStdString());
    //    if (!factory_ptr)
    //    {
    //        //log_msg(l_error, "failed to get factory for plugin '%s'\n", plugin_name.c_str());
    //        return;
    //    }

    //    mPlugin = std::dynamic_pointer_cast<CLIPluginInterface>(factory_ptr->query_interface(PluginInterfaceType::cli));
    //    if (!mPlugin)
    //    {
    //        //log_msg(l_warning, "Plugin %s is not castable to CLIPluginInterface!\n", plugin_name.c_str());
    //        return;
    //    }

    //    // END

    //    for (auto option_tupel : mPlugin->get_cli_options().get_options())
    //    {
    //        QString name = QString::fromStdString(std::get<0>(option_tupel).at(0));
    //        QString mDescription = QString::fromStdString(std::get<1>(option_tupel));
    //        QPushButton* button = new QPushButton(name, this);
    //        button->setToolTip(mDescription);
    //        button->setCheckable(true);
    //        QLineEdit* LineEdit = new QLineEdit(this);
    //        mVector.append(QPair<QPushButton*, QLineEdit*>(button, LineEdit));
    //        mFormLayout->addRow(button, LineEdit);
    //        LineEdit->setDisabled(true);
    //        connect(button, &QPushButton::toggled, LineEdit, &QLineEdit::setEnabled);
    //    }
    //}

    void PluginArgumentsWidget::setup(const QString& plugin_name)
    {
        for (QPair<QPushButton*, QLineEdit*> pair : mVector)
        {
            pair.first->deleteLater();
            pair.second->deleteLater();
        }
        mVector.clear();

        mPlugin = plugin_manager::get_plugin_instance<CLIPluginInterface>(plugin_name.toStdString(), false);
        if (!mPlugin)
        {
            //log_msg(l_warning, "Plugin %s is not castable to CLIPluginInterface!\n", plugin_name.c_str());
            return;
        }

        for (auto option_tupel : mPlugin->get_cli_options().get_options())
        {
            QString name        = QString::fromStdString(*std::get<0>(option_tupel).begin());
            QString mDescription = QString::fromStdString(std::get<1>(option_tupel));
            QPushButton* button = new QPushButton(name, this);
            button->setToolTip(mDescription);
            button->setCheckable(true);
            QLineEdit* LineEdit = new QLineEdit(this);
            mVector.append(QPair<QPushButton*, QLineEdit*>(button, LineEdit));
            mFormLayout->addRow(button, LineEdit);
            LineEdit->setDisabled(true);
            connect(button, &QPushButton::toggled, LineEdit, &QLineEdit::setEnabled);
            /*LineEdit->hide();
            connect(button, &QPushButton::toggled, LineEdit, &QLineEdit::setVisible);*/
        }
    }

    void PluginArgumentsWidget::handlePluginSelected(int index)
    {
        for (QPair<QPushButton*, QLineEdit*>& pair : mVector)
        {
            pair.first->hide();
            pair.second->hide();
            mFormLayout->removeWidget(pair.first);
            mFormLayout->removeWidget(pair.second);
            pair.first->deleteLater();
            pair.second->deleteLater();
        }
        mVector.clear();

        for (auto arg : PluginScheduleManager::get_instance()->getSchedule()->at(index).second)
        {
            QPushButton* button = new QPushButton(arg.mFlag, this);
            button->setToolTip(arg.mDescription);
            button->setCheckable(true);
            button->setChecked(arg.mChecked);
            QLineEdit* LineEdit = new QLineEdit(arg.value, this);
            LineEdit->setEnabled(arg.mChecked);
            mVector.append(QPair<QPushButton*, QLineEdit*>(button, LineEdit));
            mFormLayout->addRow(button, LineEdit);
            connect(button, &QPushButton::toggled, LineEdit, &QLineEdit::setEnabled);

            connect(button, &QPushButton::clicked, this, &PluginArgumentsWidget::handleButtonClicked);
            connect(LineEdit, &QLineEdit::textEdited, this, &PluginArgumentsWidget::handleTextEdited);
        }
    }

    void PluginArgumentsWidget::handleButtonClicked(bool mChecked)
    {
        QObject* sender     = QObject::sender();
        QPushButton* button = static_cast<QPushButton*>(sender);
        QString mFlag        = button->text();

        int current_index                     = PluginScheduleManager::get_instance()->getCurrentIndex();
        QPair<QString, QList<Argument>>& pair = (*PluginScheduleManager::get_instance()->getSchedule())[current_index];
        for (Argument& arg : pair.second)
        {
            if (arg.mFlag == mFlag)
            {
                arg.mChecked = mChecked;
                return;
            }
        }
    }

    void PluginArgumentsWidget::handleTextEdited(const QString& text)
    {
        QObject* sender      = QObject::sender();
        QLineEdit* LineEdit = static_cast<QLineEdit*>(sender);
        //QString value = LineEdit->text();

        QString mFlag = "";
        for (auto& pair : mVector)
        {
            if (pair.second == LineEdit)
            {
                mFlag = pair.first->text();
                break;
            }
        }

        QPair<QString, QList<Argument>>& pair = (*PluginScheduleManager::get_instance()->getSchedule())[mCurrentIndex];
        for (Argument& arg : pair.second)
        {
            if (arg.mFlag == mFlag)
            {
                //arg.value = value;
                arg.value = text;
                return;
            }
        }
    }

    char* PluginArgumentsWidget::toHeapCstring(const QString& string)
    {
        std::string std_string = string.toStdString();
        char* cstring          = new char[std_string.size() + 1];
        std::copy(std_string.begin(), std_string.end(), cstring);
        cstring[std_string.size()] = '\0';
        return cstring;
    }
}
