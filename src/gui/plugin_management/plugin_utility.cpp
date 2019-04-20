#include "plugin_management/plugin_utility.h"

#include "core/interface_cli.h"
#include "core/interface_factory.h"
#include "core/interface_gui.h"
#include "core/log.h"
#include "core/plugin_manager.h"
#include "core/program_arguments.h"
#include "gui_globals.h"

namespace plugin_utility
{
    int run_plugin(const std::string& plugin_name, program_arguments* args)
    {
        if (!args)
        {
            log_error("gui", "Program_arguments for plugin {} is a nullpointer!", plugin_name);
            return 0;
        }

        i_factory* factory = get_plugin_factory(plugin_name);
        auto plugin        = query_plugin_cli(factory);
        if (!plugin)
            return 0;

        log_info("gui", "Running plugin {}", plugin_name);
        return plugin->handle_cli_call(g_netlist, *args);
    }

    i_factory* get_plugin_factory(const std::string& plugin_name)
    {
        i_factory* factory = plugin_manager::get_plugin_factory(plugin_name);
        if (!factory)
        {
            log_error("gui", "Failed to get factory for plugin {}", plugin_name);
            return nullptr;
        }
        return factory;
    }

    std::shared_ptr<i_cli> query_plugin_cli(i_factory* factory)
    {
        auto baseptr               = factory->query_interface(interface_type::cli);
        std::shared_ptr<i_cli> ptr = std::dynamic_pointer_cast<i_cli>(baseptr);
        //    if (!ptr)
        //        log_warning("gui", "Plugin {} is not castable to i_cli!", factory->get_name());
        return ptr;
    }

    std::shared_ptr<i_gui> query_plugin_gui(i_factory* factory)
    {
        auto baseptr               = factory->query_interface(interface_type::gui);
        std::shared_ptr<i_gui> ptr = std::dynamic_pointer_cast<i_gui>(baseptr);
        //    if (!ptr)
        //        log_warning("gui", "Plugin {} is not castable to i_gui!", factory->get_name());
        return ptr;
    }

}    // namespace plugin_utility
