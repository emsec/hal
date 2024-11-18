#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "machine_learning/features/gate_pair_feature.h"

#define MAX_DISTANCE 255

namespace hal
{
    namespace machine_learning
    {
        namespace gate_pair_feature
        {
            const Result<NetlistAbstraction*> FeatureContext::get_sequential_abstraction()
            {
                if (!m_seqential_abstraction.has_value())
                {
                    const auto seq_gates = nl->get_gates([](const auto* g) { return g->get_type()->has_property(GateTypeProperty::sequential); });

                    const std::vector<PinType> forbidden_pins = {
                        PinType::clock, /*PinType::done, PinType::error, PinType::error_detection,*/ /*PinType::none,*/ PinType::ground, PinType::power /*, PinType::status*/};

                    const auto endpoint_filter = [forbidden_pins](const auto* ep, const auto& _d) {
                        UNUSED(_d);
                        return std::find(forbidden_pins.begin(), forbidden_pins.end(), ep->get_pin()->get_type()) == forbidden_pins.end();
                    };

                    const auto sequential_abstraction_res = NetlistAbstraction::create(nl, seq_gates, true, endpoint_filter, endpoint_filter);
                    if (sequential_abstraction_res.is_error())
                    {
                        return ERR_APPEND(sequential_abstraction_res.get_error(), "cannot get sequential netlist abstraction for gate feature context: failed to build abstraction.");
                    }

                    m_seqential_abstraction = sequential_abstraction_res.get();
                }

                return OK(&(m_seqential_abstraction.value()));
            }

            Result<std::vector<u32>> LogicalDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return OK({0});
                }

                const auto res = NetlistTraversalDecorator(*fc.nl).get_shortest_path_distance(g_a, g_b, m_direction);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                }

                const auto shortest_path = res.get();

                if (!shortest_path.has_value())
                {
                    return OK({MAX_DISTANCE});
                }

                if (shortest_path.value() < 1)
                {
                    log_error("machine_learning", "Found shortest path with no components, this is unexpected!");
                }

                const u32 distance = shortest_path.value();
                return OK({std::min(distance, u32(MAX_DISTANCE))});
            };

            std::string LogicalDistance::to_string() const
            {
                return "LogicalDistance";
            }

            Result<std::vector<u32>> SequentialDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return OK({0});
                }

                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": : failed to get sequential netlist abstraction");
                }

                const auto res = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_a, g_b, m_direction);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                }

                const auto shortest_path = res.get();

                if (!shortest_path.has_value())
                {
                    return OK({MAX_DISTANCE});
                }

                if (shortest_path.value() < 1)
                {
                    log_error("machine_learning", "Found shortest path with only one component, this is unexpected!");
                }

                const u32 distance = shortest_path.value();
                return OK({std::min(distance, u32(MAX_DISTANCE))});
            };

            std::string SequentialDistance::to_string() const
            {
                return "SequentialDistance";
            }

            Result<std::vector<u32>> PhysicalDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                log_error("machine_learning", "Physical distance currently not implemented as gate pair feature.");

                return OK({});
            };

            std::string PhysicalDistance::to_string() const
            {
                return "PhysicalDistance";
            }

            Result<std::vector<u32>> SharedControlSignals::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                static const std::vector<PinType> ctrl_pin_types = {
                    PinType::clock,
                    PinType::enable,
                    PinType::reset,
                    PinType::set,
                };

                std::set<Net*> nets_a;
                std::set<Net*> nets_b;

                for (const auto& t : ctrl_pin_types)
                {
                    for (const auto& pin : g_a->get_type()->get_pins([t](const GatePin* p) { return p->get_type() == t; }))
                    {
                        nets_a.insert(g_a->get_fan_in_net(pin));
                    }

                    for (const auto& pin : g_b->get_type()->get_pins([t](const GatePin* p) { return p->get_type() == t; }))
                    {
                        nets_b.insert(g_b->get_fan_in_net(pin));
                    }
                }

                std::set<Net*> shared;
                std::set_intersection(nets_a.begin(), nets_a.end(), nets_b.begin(), nets_b.end(), std::inserter(shared, shared.begin()));

                return OK({u32(shared.size())});
            };

            std::string SharedControlSignals::to_string() const
            {
                return "SharedControlSignals";
            }

            Result<std::vector<u32>> SharedSequentialNeighbors::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                // TODO add caching
                std::unordered_map<const Net*, std::set<Gate*>>* cache;

                const auto get_n_next_sequential_gates = [&fc, &cache](const Gate* g, const bool direction, const u32 depth) -> Result<std::set<Gate*>> {
                    std::set<Gate*> total_neighbors;

                    std::vector<const Gate*> current_neighbors = {g};
                    std::vector<const Gate*> next_neighbors;

                    for (u32 i = 0; i < depth; i++)
                    {
                        u32 prev_size = total_neighbors.size();

                        for (const auto& g : current_neighbors)
                        {
                            const auto& res = NetlistTraversalDecorator(*fc.nl).get_next_sequential_gates(g, direction);
                            if (res.is_error())
                            {
                                return res;
                                // return ERR_APPEND(res.get_error(), "cannot caluclate feature " + to_string() + ": failed to get next sequential gates.");
                            }
                            total_neighbors.insert(res.get().begin(), res.get().end());
                            next_neighbors.insert(next_neighbors.end(), res.get().begin(), res.get().end());
                        }

                        if (prev_size == total_neighbors.size())
                        {
                            break;
                        }

                        current_neighbors = next_neighbors;
                        next_neighbors.clear();
                    }

                    return OK(total_neighbors);
                };

                std::set<const Gate*> neighbors_a = g_a->get_type()->has_property(GateTypeProperty::sequential) ? std::set<const Gate*>{g_a} : std::set<const Gate*>{};
                std::set<const Gate*> neighbors_b = g_b->get_type()->has_property(GateTypeProperty::sequential) ? std::set<const Gate*>{g_b} : std::set<const Gate*>{};

                if (m_direction == PinDirection::output)
                {
                    const auto res_a = get_n_next_sequential_gates(g_a, true, m_depth);
                    if (res_a.is_error())
                    {
                        return ERR_APPEND(res_a.get_error(), "cannot calculate feature " + to_string() + ": failed to get next sequnetial gates.");
                    }

                    for (const auto g_n : res_a.get())
                    {
                        neighbors_a.insert(g_n);
                    }

                    const auto res_b = get_n_next_sequential_gates(g_b, true, m_depth);
                    if (res_b.is_error())
                    {
                        return ERR_APPEND(res_b.get_error(), "cannot calculate feature " + to_string() + ": failed to get next sequnetial gates.");
                    }

                    for (const auto g_n : res_b.get())
                    {
                        neighbors_b.insert(g_n);
                    }
                }

                if (m_direction == PinDirection::input)
                {
                    const auto res_a = get_n_next_sequential_gates(g_a, false, m_depth);
                    if (res_a.is_error())
                    {
                        return ERR_APPEND(res_a.get_error(), "cannot calculate feature " + to_string() + ": failed to get next sequnetial gates.");
                    }

                    for (const auto g_n : res_a.get())
                    {
                        neighbors_a.insert(g_n);
                    }

                    const auto res_b = get_n_next_sequential_gates(g_b, false, m_depth);
                    if (res_b.is_error())
                    {
                        return ERR_APPEND(res_b.get_error(), "cannot calculate feature " + to_string() + ": failed to get next sequnetial gates.");
                    }

                    for (const auto g_n : res_b.get())
                    {
                        neighbors_b.insert(g_n);
                    }
                }

                if (m_direction == PinDirection::inout)
                {
                    // NOTE this is either trivial by just combining both directions or more complex by building a real undirected graph (predecessors of a successor would also be neighbors eventhough the direction siwtches)
                    return ERR("cannot calculate feature " + to_string() + ": NOT IMPLEMENTED REACHED.");
                }

                std::set<const Gate*> shared;
                std::set_intersection(neighbors_a.begin(), neighbors_a.end(), neighbors_b.begin(), neighbors_b.end(), std::inserter(shared, shared.begin()));

                return OK({u32(shared.size())});
            };

            std::string SharedSequentialNeighbors::to_string() const
            {
                return "SharedSequentialNeighbors";
            }

            Result<std::vector<u32>> SharedNeighbors::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                std::set<const Gate*> neighbors_a = {g_a};
                std::set<const Gate*> neighbors_b = {g_b};

                if (m_direction == PinDirection::output || m_direction == PinDirection::input)
                {
                    const bool search_successors = m_direction == PinDirection::output;

                    auto subgraph_a_res = NetlistTraversalDecorator(*fc.nl).get_next_matching_gates_until_depth(g_a, search_successors, m_depth);
                    auto subgraph_b_res = NetlistTraversalDecorator(*fc.nl).get_next_matching_gates_until_depth(g_b, search_successors, m_depth);

                    if (subgraph_a_res.is_error())
                    {
                        return ERR_APPEND(subgraph_a_res.get_error(), "cannot calculate feature " + to_string() + ": failed to get next matching gates.");
                    }

                    if (subgraph_b_res.is_error())
                    {
                        return ERR_APPEND(subgraph_b_res.get_error(), "cannot calculate feature " + to_string() + ": failed to get next matching gates.");
                    }

                    for (const auto g_n : subgraph_a_res.get())
                    {
                        neighbors_a.insert(g_n);
                    }

                    for (const auto g_n : subgraph_b_res.get())
                    {
                        neighbors_b.insert(g_n);
                    }

                    // TODO remove debug print
                    // std::cout << "Subgraph A: " << std::endl;
                    // for (const auto& g_a : neighbors_a)
                    // {
                    //     std::cout << "\t" << g_a->get_id() << " / " << g_a->to_string() << std::endl;
                    // }

                    // std::cout << "Subgraph B: " << std::endl;
                    // for (const auto& g_b : neighbors_b)
                    // {
                    //     std::cout << "\t" << g_b->get_id() << " / " << g_b->to_string() << std::endl;
                    // }
                }

                if (m_direction == PinDirection::inout)
                {
                    // NOTE this is either trivial by just combining both directions or more complex by building a real undirected graph (predecessors of a successor would also be neighbors eventhough the direction switches)
                    return ERR("cannot calculate feature " + to_string() + ": NOT IMPLEMENTED REACHED.");
                }

                std::set<const Gate*> shared;
                std::set_intersection(neighbors_a.begin(), neighbors_a.end(), neighbors_b.begin(), neighbors_b.end(), std::inserter(shared, shared.begin()));

                return OK({u32(shared.size())});
            };

            std::string SharedNeighbors::to_string() const
            {
                return "SharedNeighbors";
            }

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
            {
                FeatureContext fc(g_a->get_netlist());
                return build_feature_vec(fc, features, g_a, g_b);
            }

            Result<std::vector<u32>> build_feature_vec(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
            {
                std::vector<u32> feature_vec;

                for (const auto& gf : features)
                {
                    // TODO remove
                    // std::cout << "Calculating feature " << gf->to_string() << " for g_a " << g_a->get_id() << " and g_b " << g_b->get_id() << std::endl;

                    const auto new_features = gf->calculate_feature(fc, g_a, g_b);
                    if (new_features.is_error())
                    {
                        return ERR_APPEND(new_features.get_error(),
                                          "cannot build feature vector for gate_a " + g_a->get_name() + " with ID " + std::to_string(g_a->get_id()) + " and gate_b " + g_b->get_name() + " with ID "
                                              + std::to_string(g_b->get_id()) + ": failde to calculate feature " + gf->to_string());
                    }

                    feature_vec.insert(feature_vec.end(), new_features.get().begin(), new_features.get().end());
                }

                return OK(feature_vec);
            }

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const std::pair<const Gate*, const Gate*>& gate_pair)
            {
                return build_feature_vec(features, gate_pair.first, gate_pair.second);
            }

            Result<std::vector<u32>> build_feature_vec(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const std::pair<const Gate*, const Gate*>& gate_pair)
            {
                return build_feature_vec(fc, features, gate_pair.first, gate_pair.second);
            }

            Result<std::vector<std::vector<u32>>> build_feature_vecs(const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<const Gate*, const Gate*>>& gate_pairs)
            {
                if (gate_pairs.empty())
                {
                    return OK({});
                }

                FeatureContext fc(gate_pairs.front().first->get_netlist());
                return build_feature_vecs(fc, features, gate_pairs);
            }

            Result<std::vector<std::vector<u32>>>
                build_feature_vecs(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<const Gate*, const Gate*>>& gate_pairs)
            {
                std::vector<std::vector<u32>> feature_vecs;

                for (const auto& gp : gate_pairs)
                {
                    const auto feature_vec = build_feature_vec(fc, features, gp);
                    if (feature_vec.is_error())
                    {
                        return ERR_APPEND(feature_vec.get_error(),
                                          "cannot build feature vecs: failed to build feature vector for gate_a " + gp.first->get_name() + " with ID " + std::to_string(gp.first->get_id())
                                              + " and gate_b " + gp.second->get_name() + " with ID " + std::to_string(gp.second->get_id()));
                    }

                    feature_vecs.push_back(feature_vec.get());
                }

                return OK(feature_vecs);
            }
        }    // namespace gate_pair_feature
    }    // namespace machine_learning
}    // namespace hal