#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistPreprocessingPlugin>();
    }

    std::string NetlistPreprocessingPlugin::get_name() const
    {
        return std::string("netlist_preprocessing");
    }

    std::string NetlistPreprocessingPlugin::get_version() const
    {
        return std::string("0.2");
    }

    std::string NetlistPreprocessingPlugin::get_description() const
    {
        return "A collection of tools to preprocess a netlist and prepare it for further analysis.";
    }

    std::set<std::string> NetlistPreprocessingPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("resynthesis");
        return retval;
    }
}    // namespace hal
