#include "gui/plugin_access_manager/plugin_access_manager.h"
#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_interface_gui.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/program_arguments.h"
#include "gui/gui_globals.h"
#include "gui/plugin_access_manager/extended_cli_dialog.h"

namespace hal
{
    ProgramArguments plugin_access_manager::requestArguments(const std::string plugin_name)
    {
        auto pl = plugin_manager::get_plugin_instance<BasePluginInterface>(plugin_name, false);
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
            auto plugin = dynamic_cast<CLIPluginInterface*>(pl);
            plugin->initialize();
            ExtendedCliDialog dialog(QString::fromStdString(plugin_name));
            dialog.exec();
            return dialog.getArgs();
        }
        else if (selection == PluginInterfaceType::gui)
        {
            auto plugin = dynamic_cast<GUIPluginInterface*>(pl);
            plugin->initialize();

            // show gui dialog, return results
            return ProgramArguments();
        }

        return ProgramArguments();
    }

    int plugin_access_manager::runPlugin(const std::string plugin_name, ProgramArguments* args)
    {
        if (args == nullptr)
        {
            log_error("gui", "Program_arguments for plugin {} is a nullpointer!", plugin_name);
            return 0;
        }

        auto plugin = plugin_manager::get_plugin_instance<CLIPluginInterface>(plugin_name);
        if (!plugin)
            return 0;

        log_info("gui", "Running plugin {}", plugin_name);
        return plugin->handle_cli_call(gNetlist, *args);
    }
}
