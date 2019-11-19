#include "hal_plugin_access_manager/hal_plugin_access_manager.h"
#include "core/interface_cli.h"
#include "core/interface_gui.h"
#include "core/plugin_manager.h"
#include "core/program_arguments.h"
#include "gui_globals.h"
#include "hal_plugin_access_manager/hal_extended_cli_dialog.h"

program_arguments hal_plugin_access_manager::request_arguments(const std::string plugin_name)
{
    auto pl = plugin_manager::get_plugin_instance<i_base>(plugin_name, false);
    if (!pl)
    {
        return program_arguments();
    }

    interface_type selection;
    if (pl->has_type(interface_type::cli))
    {
        if (pl->has_type(interface_type::gui))
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
        if (pl->has_type(interface_type::gui))
            selection = interface_type::gui;
        else
            return program_arguments();
    }

    if (selection == interface_type::cli)
    {
        auto plugin = std::dynamic_pointer_cast<i_cli>(pl);
        plugin->initialize();
        hal_extended_cli_dialog dialog(QString::fromStdString(plugin_name));
        dialog.exec();
        return dialog.get_args();
    }
    else if (selection == interface_type::gui)
    {
        auto plugin = std::dynamic_pointer_cast<i_gui>(pl);
        plugin->initialize();

        // show gui dialog, return results
        return program_arguments();
    }

    return program_arguments();
}

int hal_plugin_access_manager::run_plugin(const std::string plugin_name, program_arguments* args)
{
    if (args == nullptr)
    {
        log_error("gui", "Program_arguments for plugin {} is a nullpointer!", plugin_name);
        return 0;
    }

    auto plugin = plugin_manager::get_plugin_instance<i_cli>(plugin_name);
    if (!plugin)
        return 0;

    log_info("gui", "Running plugin {}", plugin_name);
    return plugin->handle_cli_call(g_netlist, *args);
}
