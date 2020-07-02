#include "plugin_access_manager/plugin_access_manager.h"
#include "core/plugin_interface_cli.h"
#include "core/plugin_interface_gui.h"
#include "core/plugin_manager.h"
#include "core/program_arguments.h"
#include "gui_globals.h"
#include "plugin_access_manager/extended_cli_dialog.h"

namespace hal
{
    ProgramArguments plugin_access_manager::request_arguments(const std::string plugin_name)
    {
        auto pl = PluginManager::get_plugin_instance<BasePluginInterface>(plugin_name, false);
        if (!pl)
        {
            return ProgramArguments();
        }

        PluginInterfaceType selection;
        if (pl->has_type(PluginInterfaceType::cli))
        {
            if (pl->has_type(PluginInterfaceType::gui))
            {
                // show selection dialog
                //Workaround:
                selection = PluginInterfaceType::cli;
            }
            else
                selection = PluginInterfaceType::cli;
        }
        else
        {
            if (pl->has_type(PluginInterfaceType::gui))
                selection = PluginInterfaceType::gui;
            else
                return ProgramArguments();
        }

        if (selection == PluginInterfaceType::cli)
        {
            auto plugin = std::dynamic_pointer_cast<CLIPluginInterface>(pl);
            plugin->initialize();
            ExtendedCliDialog dialog(QString::fromStdString(plugin_name));
            dialog.exec();
            return dialog.get_args();
        }
        else if (selection == PluginInterfaceType::gui)
        {
            auto plugin = std::dynamic_pointer_cast<GUIPluginInterface>(pl);
            plugin->initialize();

            // show gui dialog, return results
            return ProgramArguments();
        }

        return ProgramArguments();
    }

    int plugin_access_manager::run_plugin(const std::string plugin_name, ProgramArguments* args)
    {
        if (args == nullptr)
        {
            log_error("gui", "Program_arguments for plugin {} is a nullpointer!", plugin_name);
            return 0;
        }

        auto plugin = PluginManager::get_plugin_instance<CLIPluginInterface>(plugin_name);
        if (!plugin)
            return 0;

        log_info("gui", "Running plugin {}", plugin_name);
        return plugin->handle_cli_call(g_netlist, *args);
    }
}
