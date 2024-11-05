#pragma once
#include "hal_core/defines.h"
#include "machine_learning/types.h"

#include <vector>

namespace hal
{
    class Netlist;

    namespace machine_learning
    {
        namespace graph
        {
            struct NetlistGraph
            {
                std::pair<std::vector<u32>, std::vector<u32>> edge_list;
                GraphDirection direction;
            };

            NetlistGraph construct_netlist_graph(const Netlist* nl, const std::vector<Gate*>& gates, const GraphDirection& dir);

            void annotate_netlist_graph(Netlist* nl, const std::vector<Gate*>& gates, const NetlistGraph& nlg, const std::vector<std::vector<u32>>& node_features);
        }    // namespace graph
    }    // namespace machine_learning
}    // namespace hal