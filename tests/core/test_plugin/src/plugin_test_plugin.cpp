#include "plugin_test_plugin.h"
#include "core/log.h"

extern std::shared_ptr<BasePluginInterface> get_plugin_instance()
{
    return std::dynamic_pointer_cast<BasePluginInterface>(std::make_shared<plugin_test_plugin>());
}

std::string plugin_test_plugin::get_name() const
{
    return std::string("test_plugin");
}

std::string plugin_test_plugin::get_version() const
{
    return std::string("1.2.3");
}

/*
std::set<PluginInterfaceType> plugin_test_plugin::get_type()
{
    return std::set<PluginInterfaceType>{PluginInterfaceType::base, PluginInterfaceType::cli};
}
*/

ProgramOptions plugin_test_plugin::get_cli_options() const
{
    ProgramOptions description;
    description.add("--option_one", "option_one_description");
    description.add("--option_two", "option_two_description");
    return description;
}

bool plugin_test_plugin::handle_cli_call(std::shared_ptr<netlist> nl, ProgramArguments& args)
{
    UNUSED(nl);
    UNUSED(args);
    return true;
}
