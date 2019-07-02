#include "plugin_management/plugin_schedule_manager.h"

#include "core/interface_cli.h"
#include "core/interface_factory.h"
#include "core/program_arguments.h"
#include "plugin_management/plugin_arguments_widget.h"
#include "plugin_management/plugin_utility.h"

plugin_schedule_manager* plugin_schedule_manager::get_instance()
{
    static plugin_schedule_manager* manager = new plugin_schedule_manager();
    return manager;

    // IMPLEMENT INDEX UPDATES (OR CHANGE ARCHITECTURE)
}

plugin_schedule_manager::schedule* plugin_schedule_manager::get_schedule()
{
    return &m_schedule;
}

program_arguments plugin_schedule_manager::get_program_arguments(int index)
{
    program_arguments args;

    for (argument arg : m_schedule.at(index).second)
    {
        std::vector<std::string> vector;

        for (QString& qstring : arg.value.split(QRegExp("[\\s,;]*"), QString::SkipEmptyParts))
            vector.push_back(qstring.toStdString());

        if (arg.checked)
            args.set_option(arg.flag.toStdString(), vector);
    }
    return args;
}

int plugin_schedule_manager::get_current_index()
{
    return m_current_index;
}

void plugin_schedule_manager::set_current_index(int index)
{
    m_current_index = index;
}

void plugin_schedule_manager::add_plugin(const QString& plugin, int index)
{
    std::shared_ptr<i_cli> cli = plugin_utility::query_plugin_cli(plugin_utility::get_plugin_factory(plugin.toStdString()));

    if (!cli)
        return;

    QList<argument> list;

    for (auto option_tupel : cli->get_cli_options().get_options())
    {
        argument arg;
        arg.flag        = QString::fromStdString(*std::get<0>(option_tupel).begin());
        arg.description = QString::fromStdString(std::get<1>(option_tupel));
        arg.value       = "";
        arg.checked     = false;
        list.append(arg);
    }

    m_schedule.insert(index, QPair<QString, QList<argument>>(plugin, list));
}

void plugin_schedule_manager::move_plugin(int from, int to)
{
    m_schedule.move(from, to);
}

void plugin_schedule_manager::remove_plugin(int index)
{
    m_schedule.removeAt(index);
}

void plugin_schedule_manager::run_schedule()
{
    for (int i = 0; i < m_schedule.length(); i++)
    {
        program_arguments args = get_program_arguments(i);
        plugin_utility::run_plugin(m_schedule.at(i).first.toStdString(), &args);
    }
}

plugin_schedule_manager::plugin_schedule_manager(QObject* parent) : QObject(parent), m_current_index(0)
{
}
