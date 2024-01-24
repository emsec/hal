#include "bitwuzla_utils/plugin_bitwuzla_utils.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<BitwuzlaUtilsPlugin>();
    }

    std::string BitwuzlaUtilsPlugin::get_name() const
    {
        return std::string("bitwuzla_utils");
    }

    std::string BitwuzlaUtilsPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void BitwuzlaUtilsPlugin::initialize()
    {
    }
}    // namespace hal
