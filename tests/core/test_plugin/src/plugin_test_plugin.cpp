#include "plugin_test_plugin.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PluginTestPlugin>();
    }

    std::string PluginTestPlugin::get_name() const
    {
        return std::string("test_plugin");
    }

    std::string PluginTestPlugin::get_version() const
    {
        return std::string("1.2.3");
    }


    ProgramOptions PluginTestPlugin::get_cli_options() const
    {
        ProgramOptions description;
        description.add("--option_one", "option_one_description");
        description.add("--option_two", "option_two_description");
        return description;
    }

    bool PluginTestPlugin::handle_cli_call(Netlist* nl, ProgramArguments& args)
    {
        UNUSED(nl);
        UNUSED(args);
        return true;
    }

}    // namespace hal
