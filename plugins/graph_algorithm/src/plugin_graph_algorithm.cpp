#include "graph_algorithm/plugin_graph_algorithm.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<GraphAlgorithmPlugin>();
    }

    std::string GraphAlgorithmPlugin::get_name() const
    {
        return std::string("graph_algorithm");
    }

    std::string GraphAlgorithmPlugin::get_version() const
    {
        return std::string("0.1");
    }
}    // namespace hal
