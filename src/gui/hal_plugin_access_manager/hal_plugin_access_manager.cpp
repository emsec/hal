#include "hal_plugin_access_manager/hal_plugin_access_manager.h"
#include "core/interface_cli.h"
#include "core/interface_factory.h"
#include "core/interface_gui.h"
#include "core/plugin_manager.h"
#include "core/program_arguments.h"
#include "gui_globals.h"
#include "hal_plugin_access_manager/hal_extended_cli_dialog.h"

program_arguments hal_plugin_access_manager::request_arguments(const std::string plugin_name)
{
    i_factory* factory = get_plugin_factory(plugin_name);
    auto types         = factory->get_plugin_types();
    interface_type selection;
    if (types.find(interface_type::cli) != types.end())
    {
        if (types.find(interface_type::gui) != types.end())
        {
            // show selection dialog
            //Workaround:
            selection = interface_type::cli;
        }
        else
            selection = interface_type::cli;
    }
    else
    {
        if (types.find(interface_type::gui) != types.end())
            selection = interface_type::gui;
        else
            return program_arguments();
    }

    if (selection == interface_type::cli)
    {
        auto plugin = query_plugin_cli(factory);
        if (!plugin)
            return program_arguments();

        hal_extended_cli_dialog dialog(QString::fromStdString(plugin_name));
        dialog.exec();
        return dialog.get_args();
    }
    else
    {
        auto plugin = query_plugin_gui(factory);
        if (!plugin)
            return program_arguments();

        // show gui dialog, return results
        return program_arguments();
    }
}

int hal_plugin_access_manager::run_plugin(const std::string plugin_name, program_arguments* args)
{
    if (args == nullptr)
    {
        //log_msg(l_warning, "The variables_map for plugin %s is a nullpointer\n", plugin_name.c_str());
        return 0;
    }

    i_factory* factory = get_plugin_factory(plugin_name);
    auto plugin        = query_plugin_cli(factory);
    if (!plugin)
        return 0;

    //log_info("Running plugin: %s\n", plugin_name.c_str());
    return plugin->handle_cli_call(g_netlist, *args);
}

hal_plugin_access_manager::hal_plugin_access_manager()
{
    //private constructor to provent instantiation
}

i_factory* hal_plugin_access_manager::get_plugin_factory(std::string plugin_name)
{
    i_factory* factory = plugin_manager::get_plugin_factory(plugin_name);
    if (factory == nullptr)
    {
        //log_msg(l_error, "failed to get factory for plugin '%s'\n", plugin_name.c_str());
        return nullptr;
    }
    return factory;
}

std::shared_ptr<i_cli> hal_plugin_access_manager::query_plugin_cli(i_factory* factory)
{
    auto baseptr               = factory->query_interface(interface_type::cli);
    std::shared_ptr<i_cli> ptr = std::dynamic_pointer_cast<i_cli>(baseptr);
    //    if (!ptr)
    //        log_msg(l_warning, "Plugin %s is not castable to i_cli!\n", factory->get_name().c_str());
    return ptr;
}

std::shared_ptr<i_gui> hal_plugin_access_manager::query_plugin_gui(i_factory* factory)
{
    auto baseptr               = factory->query_interface(interface_type::gui);
    std::shared_ptr<i_gui> ptr = std::dynamic_pointer_cast<i_gui>(baseptr);
    //    if (!ptr)
    //        log_msg(l_warning, "Plugin %s is not castable to i_gui!\n", factory->get_name().c_str());
    return ptr;
}
