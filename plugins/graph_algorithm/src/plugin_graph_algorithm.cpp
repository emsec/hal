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
        return std::string("0.2");
    }

    std::string GraphAlgorithmPlugin::get_description() const
    {
        return "Graph algorithms based on igraph operating on a netlist graph abstraction.";
    }

    std::set<std::string> GraphAlgorithmPlugin::get_dependencies() const
    {
        return {};
    }
}    // namespace hal
