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
        namespace gnn
        {
            NetlistGraph construct_netlist_graph(const Netlist* nl, const std::vector<const gate_feature::GateFeature*>& node_features, const GraphDirection& dir)
            {
                std::unordered_map<const Gate*, u32> gate_to_idx;
                // init gate to index mapping
                for (u32 g_idx = 0; g_idx < nl->get_gates().size(); g_idx++)
                {
                    const Gate* g = nl->get_gates().at(g_idx);
                    gate_to_idx.insert({g, g_idx});
                }

                // edge list
                std::vector<u32> sources;
                std::vector<u32> destinations;

                for (const auto& g : nl->get_gates())
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

                // gate features
                gate_feature::FeatureContext fc = gate_feature::FeatureContext{nl};
                std::vector<std::vector<u32>> gate_features;

                for (const auto& g : nl->get_gates())
                {
                    const u32 g_idx = gate_to_idx.at(g);

                    std::vector<u32> feature_vec;

                    for (const auto& gf : node_features)
                    {
                        const auto new_features = gf->calculate_feature(fc, g);
                        feature_vec.insert(feature_vec.end(), new_features.begin(), new_features.end());
                    }

                    gate_features.push_back(feature_vec);
                }

                return {{sources, destinations}, gate_features};
            }

            std::vector<std::vector<u32>> construct_node_labels(const Netlist* nl)
            {
                // TODO this is temporary and must be replaced in the future. This is purely meant for minimal testing

                std::vector<std::vector<u32>> labels;

                for (const auto& g : nl->get_gates())
                {
                    u32 input_io_degree  = g->get_fan_in_nets().size();
                    u32 output_io_degree = g->get_fan_out_nets().size();

                    labels.push_back(std::vector<u32>{(input_io_degree + output_io_degree) > 3 ? u32(1) : u32(0)});
                }

                return labels;
            }

            void annotate_netlist_graph(Netlist* nl, const NetlistGraph& nlg)
            {
                const auto gates = nl->get_gates();

                for (u32 g_idx = 0; g_idx < gates.size(); g_idx++)
                {
                    gates.at(g_idx)->set_data("netlist_graph", "gate_index", "string", std::to_string(g_idx));

                    const auto feature_vec = nlg.node_features.at(g_idx);
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

            // NOTE only here for testing as long as the Gate Features do not have python pindings
            NetlistGraph test_construct_netlist_graph(const Netlist* nl)
            {
                // TODO think about how this can be done more beautifully, because this can not be the best solution (think about creating a unique ptr and casting this to a unqiue ptr of the parent class)
                const auto t0 = gate_feature::ConnectedGlobalIOs();
                const auto t1 = gate_feature::IODegrees();

                const std::vector<const gate_feature::GateFeature*> gf = {dynamic_cast<const gate_feature::GateFeature*>(&t0), dynamic_cast<const gate_feature::GateFeature*>(&t1)};
                return construct_netlist_graph(nl, gf, GraphDirection::bidirectional);
            }

            std::vector<std::vector<u32>> test_construct_node_labels(const Netlist* nl)
            {
                return construct_node_labels(nl);
            }
        }    // namespace gnn
    }        // namespace machine_learning
}    // namespace hal