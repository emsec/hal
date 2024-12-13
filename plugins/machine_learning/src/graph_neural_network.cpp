#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/utils.h"
#include "machine_learning/features/gate_feature.h"
#include "machine_learning/graph_neural_network.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        NetlistGraph construct_netlist_graph(const Netlist* nl, const std::vector<Gate*>& gates, const GraphDirection& dir)
        {
            UNUSED(nl);

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
                if (dir == GraphDirection::directed)
                {
                    for (const auto& pre : g->get_unique_predecessors())
                    {
                        sources.push_back(gate_to_idx.at(pre));
                        destinations.push_back(g_idx);
                    }
                }

                if (dir == GraphDirection::undirected)
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

        Result<NetlistGraph> construct_sequential_netlist_graph(const Netlist* nl, const std::vector<Gate*>& gates, const GraphDirection& dir)
        {
            std::unordered_map<const Gate*, u32> gate_to_idx;
            std::vector<Gate*> sequential_gates;

            // init gate to index mapping
            for (u32 g_idx = 0; g_idx < gates.size(); g_idx++)
            {
                Gate* g = gates.at(g_idx);
                // if (!g->get_type()->has_property(GateTypeProperty::sequential))
                // {
                //     return ERR("cannot construct sequential netlist graph: gates contain non sequential gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                // }
                if (g->get_type()->has_property(GateTypeProperty::sequential))
                {
                    sequential_gates.push_back(g);
                }

                gate_to_idx.insert({g, g_idx});
            }

            const std::vector<PinType> forbidden_pins = {
                PinType::clock, /*PinType::done, PinType::error, PinType::error_detection,*/ /*PinType::none,*/ PinType::ground, PinType::power /*, PinType::status*/};

            const auto endpoint_filter = [forbidden_pins](const auto* ep, const auto& _d) {
                UNUSED(_d);
                return std::find(forbidden_pins.begin(), forbidden_pins.end(), ep->get_pin()->get_type()) == forbidden_pins.end();
            };

            const auto sequential_abstraction_res = NetlistAbstraction::create(nl, sequential_gates, false, endpoint_filter, endpoint_filter);
            if (sequential_abstraction_res.is_error())
            {
                return ERR_APPEND(sequential_abstraction_res.get_error(), "cannot get sequential netlist abstraction for gate feature context: failed to build abstraction.");
            }
            const auto& sequential_abstraction = sequential_abstraction_res.get();

            // edge list
            std::vector<u32> sources;
            std::vector<u32> destinations;

            for (const auto& g : sequential_gates)
            {
                const u32 g_idx = gate_to_idx.at(g);
                if (dir == GraphDirection::directed)
                {
                    const auto unique_predecessors = sequential_abstraction->get_unique_predecessors(g);
                    if (unique_predecessors.is_error())
                    {
                        return ERR_APPEND(unique_predecessors.get_error(),
                                          "cannot construct sequential netlist graph: failed to find unique predecessors for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }
                    for (const auto& pre : unique_predecessors.get())
                    {
                        sources.push_back(gate_to_idx.at(pre));
                        destinations.push_back(g_idx);
                    }
                }

                if (dir == GraphDirection::undirected)
                {
                    const auto unique_successors = sequential_abstraction->get_unique_successors(g);
                    if (unique_successors.is_error())
                    {
                        return ERR_APPEND(unique_successors.get_error(),
                                          "cannot construct sequential netlist graph: failed to find unique successors for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }
                    for (const auto& suc : unique_successors.get())
                    {
                        sources.push_back(g_idx);
                        destinations.push_back(gate_to_idx.at(suc));

                        sources.push_back(gate_to_idx.at(suc));
                        destinations.push_back(g_idx);
                    }
                }
            }

            return OK({{sources, destinations}, dir});
        }

        void annotate_netlist_graph(Netlist* nl, const std::vector<Gate*>& gates, const NetlistGraph& nlg, const std::vector<std::vector<u32>>& node_features)
        {
            UNUSED(nl);

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

            for (const auto& [src, dsts] : edges)
            {
                const auto vec_str = utils::join(", ", dsts.begin(), dsts.end(), [](const u32 u) { return std::to_string(u); });
                gates.at(src)->set_data("netlist_graph", "destinations", "string", vec_str);
            }
        }

    }    // namespace machine_learning
}    // namespace hal