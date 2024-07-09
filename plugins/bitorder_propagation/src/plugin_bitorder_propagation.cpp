#include "bitorder_propagation/plugin_bitorder_propagation.h"

// #define PRINT_CONFLICT
// #define PRINT_CONNECTIVITY
// // #define PRINT_CONNECTIVITY_BUILDING
// #define PRINT_GENERAL

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<BitorderPropagationPlugin>();
    }

    std::string BitorderPropagationPlugin::get_name() const
    {
        return std::string("bitorder_propagation");
    }

    std::string BitorderPropagationPlugin::get_version() const
    {
        return std::string("0.2");
    }

    std::string BitorderPropagationPlugin::get_description() const
    {
        return "Tool to automatically propagate known bit orders to module pin groups of unknown bit order.";
    }

}    // namespace hal