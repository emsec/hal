#include "machine_learning/graph_neural_network.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/utils.h"
#include "machine_learning/features/gate_feature.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace graph
        {
            NetlistGraph construct_netlist_graph(const Netlist* nl, const std::vector<Gate*>& gates, const GraphDirection& dir)
            {
                std::unordered_map<const Gate*, u32> gate_to_idx;
                // init gate to index mapping
                for (u32 g_idx = 0; g_idx < gates.size(); g_idx++)
                {
                    const Gate* g = gates.at(g_idx);
                    gate_to_idx.insert({g, g_idx});
                }

                // edge list
                std::vector<u32> sources;
                std::vector<u32> destinations;

                for (const auto& g : gates)
                {
                    const u32 g_idx = gate_to_idx.at(g);
                    if (dir == GraphDirection::directed_backward)
                    {
                        for (const auto& pre : g->get_unique_predecessors())
                        {
                            sources.push_back(gate_to_idx.at(pre));
                            destinations.push_back(g_idx);
                        }
                    }

                    if (dir == GraphDirection::directed_forward)
                    {
                        for (const auto& suc : g->get_unique_successors())
                        {
                            sources.push_back(g_idx);
                            destinations.push_back(gate_to_idx.at(suc));
                        }
                    }

                    if (dir == GraphDirection::bidirectional)
                    {
                        for (const auto& suc : g->get_unique_successors())
                        {
                            sources.push_back(g_idx);
                            destinations.push_back(gate_to_idx.at(suc));

                            sources.push_back(gate_to_idx.at(suc));
                            destinations.push_back(g_idx);
                        }
                    }
                }

                return {{sources, destinations}, dir};
            }

            void annotate_netlist_graph(Netlist* nl, const std::vector<Gate*>& gates, const NetlistGraph& nlg, const std::vector<std::vector<u32>>& node_features)
            {
                for (u32 g_idx = 0; g_idx < gates.size(); g_idx++)
                {
                    gates.at(g_idx)->set_data("netlist_graph", "gate_index", "string", std::to_string(g_idx));

                    const auto feature_vec = node_features.at(g_idx);
                    const auto feature_str = utils::join(", ", feature_vec.begin(), feature_vec.end(), [](const u32 u) { return std::to_string(u); });

                    gates.at(g_idx)->set_data("netlist_graph", "features", "string", feature_str);
                }

                std::unordered_map<u32, std::vector<u32>> edges;
                for (u32 edge_idx = 0; edge_idx < nlg.edge_list.first.size(); edge_idx++)
                {
                    const auto src = nlg.edge_list.first.at(edge_idx);
                    const auto dst = nlg.edge_list.second.at(edge_idx);

                    edges[src].push_back(dst);
                }

                for (const auto [src, dsts] : edges)
                {
                    const auto vec_str = utils::join(", ", dsts.begin(), dsts.end(), [](const u32 u) { return std::to_string(u); });
                    gates.at(src)->set_data("netlist_graph", "destinations", "string", vec_str);
                }
            }
        }    // namespace graph
    }    // namespace machine_learning
}    // namespace hal