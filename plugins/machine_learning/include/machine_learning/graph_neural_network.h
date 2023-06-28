#pragma once
#include "hal_core/defines.h"
#include "machine_learning/types.h"

#include <vector>

namespace hal
{
    class Netlist;

    namespace machine_learning
    {
        namespace gate_feature
        {
            class GateFeature;
        }

        namespace gnn
        {
            struct NetlistGraph
            {
                std::pair<std::vector<u32>, std::vector<u32>> edge_list;
                std::vector<std::vector<u32>> node_features;
            };

            NetlistGraph construct_netlist_graph(const Netlist* nl, const std::vector<const gate_feature::GateFeature*>& node_features, const GraphDirection& dir);
            std::vector<std::vector<u32>> construct_node_labels(const Netlist* nl);

            void annotate_netlist_graph(Netlist* nl, const NetlistGraph& nlg);

            NetlistGraph test_construct_netlist_graph(const Netlist* nl);
            std::vector<std::vector<u32>> test_construct_node_labels(const Netlist* nl);
        }    // namespace gnn
    }        // namespace machine_learning
}    // namespace hal