#include "gui/plugin_management/plugin_schedule_manager.h"

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/program_arguments.h"
#include "gui/plugin_access_manager/plugin_access_manager.h"
#include "gui/plugin_management/plugin_arguments_widget.h"

namespace hal
{
    PluginScheduleManager* PluginScheduleManager::get_instance()
    {
        static PluginScheduleManager* manager = new PluginScheduleManager();
        return manager;

        // IMPLEMENT INDEX UPDATES (OR CHANGE ARCHITECTURE)
    }

    PluginScheduleManager::schedule* PluginScheduleManager::getSchedule()
    {
        return &mSchedule;
    }

    ProgramArguments PluginScheduleManager::getProgramArguments(int index)
    {
        ProgramArguments args;

        for (Argument arg : mSchedule.at(index).second)
        {
            std::vector<std::string> vector;

            for (QString& qstring : arg.value.split(QRegExp("[\\s,;]*"), QString::SkipEmptyParts))
                vector.push_back(qstring.toStdString());

            if (arg.mChecked)
                args.set_option(arg.mFlag.toStdString(), vector);
        }
        return args;
    }

    int PluginScheduleManager::getCurrentIndex()
    {
        return mCurrentIndex;
    }

    void PluginScheduleManager::setCurrentIndex(int index)
    {
        mCurrentIndex = index;
    }

    void PluginScheduleManager::addPlugin(const QString& plugin, int index)
    {
        auto cli = plugin_manager::get_plugin_instance<CLIPluginInterface>(plugin.toStdString(), false);

        if (!cli)
            return;

        QList<Argument> list;

        for (auto option_tupel : cli->get_cli_options().get_options())
        {
            Argument arg;
            arg.mFlag        = QString::fromStdString(*std::get<0>(option_tupel).begin());
            arg.mDescription = QString::fromStdString(std::get<1>(option_tupel));
            arg.value       = "";
            arg.mChecked     = false;
            list.append(arg);
        }

        mSchedule.insert(index, QPair<QString, QList<Argument>>(plugin, list));
    }

    void PluginScheduleManager::movePlugin(int from, int to)
    {
        mSchedule.move(from, to);
    }

    void PluginScheduleManager::removePlugin(int index)
    {
        mSchedule.removeAt(index);
    }

    void PluginScheduleManager::runSchedule()
    {
        for (int i = 0; i < mSchedule.length(); i++)
        {
            ProgramArguments args = getProgramArguments(i);
            plugin_access_manager::runPlugin(mSchedule.at(i).first.toStdString(), &args);
        }
    }

    PluginScheduleManager::PluginScheduleManager(QObject* parent) : QObject(parent), mCurrentIndex(0)
    {
    }
}
