#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/utils.h"
#include "machine_learning/features/gate_feature.h"
#include "machine_learning/graph_neural_network.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace
        {
            struct TupleHash
            {
                std::size_t operator()(const std::tuple<Gate*, Gate*, std::vector<FEATURE_TYPE>>& t) const
                {
                    auto& [g1, g2, features] = t;
                    std::size_t h            = reinterpret_cast<std::size_t>(g1) ^ (reinterpret_cast<std::size_t>(g2) << 1);
                    for (const auto& f : features)
                    {
                        h ^= std::hash<FEATURE_TYPE>{}(f) + 0x9e3779b9 + (h << 6) + (h >> 2);
                    }
                    return h;
                }
            };

            void dedupe_connections(std::vector<std::tuple<Gate*, Gate*, std::vector<FEATURE_TYPE>>>& connections)
            {
                std::unordered_set<std::tuple<Gate*, Gate*, std::vector<FEATURE_TYPE>>, TupleHash> seen;
                connections.erase(std::remove_if(connections.begin(), connections.end(), [&seen](const auto& conn) { return !seen.insert(conn).second; }), connections.end());
            }
        }    // namespace

        Result<NetlistGraph> construct_netlist_graph(Context& ctx, const GraphDirection& dir, const std::vector<const edge_feature::EdgeFeature*>& edge_features)
        {
            const auto& gates = ctx.get_gates();

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
            std::vector<std::vector<FEATURE_TYPE>> features;

            for (const auto& g : gates)
            {
                std::vector<std::tuple<Gate*, Gate*, std::vector<FEATURE_TYPE>>> connections;

                for (const auto* src_ep : g->get_fan_out_endpoints())
                {
                    for (const auto* dst_ep : src_ep->get_net()->get_destinations())
                    {
                        const auto ef = build_feature_vec(ctx, edge_features, src_ep, dst_ep);
                        connections.push_back({src_ep->get_gate(), dst_ep->get_gate(), ef.get()});

                        if (dir == GraphDirection::undirected)
                        {
                            const auto ef_rev = build_feature_vec(ctx, edge_features, src_ep, dst_ep);
                            connections.push_back({dst_ep->get_gate(), src_ep->get_gate(), ef_rev.get()});
                        }
                    }
                }

                // possibly dedupe
                dedupe_connections(connections);

                for (auto& [src_g, dst_g, ef] : connections)
                {
                    const auto src_idx = gate_to_idx.at(src_g);
                    const auto dst_idx = gate_to_idx.at(dst_g);

                    sources.push_back(src_idx);
                    destinations.push_back(dst_idx);
                    features.push_back(ef);
                }
            }

            return OK({{sources, destinations}, features, dir});
        }

        Result<NetlistGraph> construct_sequential_netlist_graph(Context& ctx, const GraphDirection& dir, const std::vector<const edge_feature::EdgeFeature*>& edge_features)
        {
            std::unordered_map<const Gate*, u32> gate_to_idx;
            const auto& all_gates = ctx.get_gates();

            std::vector<Gate*> sequential_gates;

            // init gate to index mapping (increment index even for non sequential gates, to get the same gate to index mapping as for the regular netlist graph)
            for (u32 g_idx = 0; g_idx < all_gates.size(); g_idx++)
            {
                Gate* g = all_gates.at(g_idx);

                if (g->get_type()->has_property(GateTypeProperty::sequential))
                {
                    sequential_gates.push_back(g);
                }

                gate_to_idx.insert({g, g_idx});
            }

            const auto sequential_abstraction_res = ctx.get_sequential_abstraction();
            if (sequential_abstraction_res.is_error())
            {
                return ERR_APPEND(sequential_abstraction_res.get_error(), "cannot get sequential netlist abstraction for gate feature context: failed to build abstraction.");
            }
            const auto& sequential_abstraction = sequential_abstraction_res.get();

            // edge list
            std::vector<u32> sources;
            std::vector<u32> destinations;
            std::vector<std::vector<FEATURE_TYPE>> features;

            for (const auto& g : sequential_gates)
            {
                std::vector<std::tuple<Gate*, Gate*, std::vector<FEATURE_TYPE>>> connections;

                for (const auto* src_ep : g->get_fan_out_endpoints())
                {
                    const auto successors = sequential_abstraction->get_successors(src_ep);
                    if (successors.is_error())
                    {
                        return ERR_APPEND(successors.get_error(),
                                          "cannot construct sequential netlist graph: failed to find successors for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    for (const auto* dst_ep : successors.get())
                    {
                        const auto ef = build_feature_vec(ctx, edge_features, src_ep, dst_ep);
                        connections.push_back({src_ep->get_gate(), dst_ep->get_gate(), ef.get()});

                        if (dir == GraphDirection::undirected)
                        {
                            const auto ef_rev = build_feature_vec(ctx, edge_features, src_ep, dst_ep);
                            connections.push_back({dst_ep->get_gate(), src_ep->get_gate(), ef_rev.get()});
                        }
                    }
                }

                // possibly dedupe
                dedupe_connections(connections);

                for (auto& [src_g, dst_g, ef] : connections)
                {
                    const auto src_idx = gate_to_idx.at(src_g);
                    const auto dst_idx = gate_to_idx.at(dst_g);

                    sources.push_back(src_idx);
                    destinations.push_back(dst_idx);
                    features.push_back(ef);
                }
            }

            return OK({{sources, destinations}, features, dir});
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