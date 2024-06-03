#include "hawkeye/plugin_hawkeye.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<HawkeyePlugin>();
    }

    std::string HawkeyePlugin::get_name() const
    {
        return std::string("hawkeye");
    }

    std::string HawkeyePlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string HawkeyePlugin::get_description() const
    {
        return "Attempts to locate arbitrary symmetric cryptographic implementations.";
    }

    std::set<std::string> HawkeyePlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("graph_algorithm");
        return retval;
    }
}    // namespace hal
