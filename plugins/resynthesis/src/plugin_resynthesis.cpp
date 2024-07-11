#include "resynthesis/plugin_resynthesis.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<ResynthesisPlugin>();
    }

    std::string ResynthesisPlugin::get_name() const
    {
        return std::string("resynthesis");
    }

    std::string ResynthesisPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string ResynthesisPlugin::get_description() const
    {
        return "Provides functions to re-synthesize (parts of) a gate-level netlist.";
    }

    std::set<std::string> ResynthesisPlugin::get_dependencies() const
    {
        return {};
    }
}    // namespace hal
