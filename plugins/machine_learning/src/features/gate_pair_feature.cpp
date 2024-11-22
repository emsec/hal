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

            const Result<NetlistAbstraction*> FeatureContext::get_original_abstraction()
            {
                if (!m_original_abstraction.has_value())
                {
                    // const std::vector<PinType> forbidden_pins = {
                    //     PinType::clock, /*PinType::done, PinType::error, PinType::error_detection,*/ /*PinType::none,*/ PinType::ground, PinType::power /*, PinType::status*/};

                    // const auto endpoint_filter = [forbidden_pins](const auto* ep, const auto& _d) {
                    //     UNUSED(_d);
                    //     return std::find(forbidden_pins.begin(), forbidden_pins.end(), ep->get_pin()->get_type()) == forbidden_pins.end();
                    // };

                    const auto original_abstraction_res = NetlistAbstraction::create(nl, nl->get_gates(), true, nullptr, nullptr);
                    if (original_abstraction_res.is_error())
                    {
                        return ERR_APPEND(original_abstraction_res.get_error(), "cannot get original netlist abstraction for gate feature context: failed to build abstraction.");
                    }

                    m_original_abstraction = original_abstraction_res.get();

                    // TODO remove debug print
                    // std::cout << "Built abstraction" << std::endl;
                }

                return OK(&m_original_abstraction.value());
            }

            Result<std::vector<u32>> LogicalDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return OK({0});
                }

                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": : failed to get original netlist abstraction");
                }

                // necessary workaround to please compiler
                const auto& forbidden_pin_types = m_forbidden_pin_types;
                const auto endpoint_filter      = [forbidden_pin_types](const auto* ep, const auto& _d) {
                    UNUSED(_d);
                    return std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) == forbidden_pin_types.end();
                };

                const auto res = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_a, g_b, m_direction, m_directed, endpoint_filter, endpoint_filter);
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

                // necessary workaround to please compiler
                const auto& forbidden_pin_types = m_forbidden_pin_types;
                const auto endpoint_filter      = [forbidden_pin_types](const auto* ep, const auto& _d) {
                    UNUSED(_d);
                    return std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) == forbidden_pin_types.end();
                };

                const auto res = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_a, g_b, m_direction, m_directed, endpoint_filter, endpoint_filter);
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
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get sequential netlist abstraction");
                }

                // fix to make compiler happpy
                const auto depth               = m_depth;
                const auto forbidden_pin_types = m_forbidden_pin_types;
                const auto starting_pin_types  = m_starting_pin_types;

                const auto endpoint_filter_a = [g_a, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g_a && !starting_pin_types.empty())
                    {
                        if (std::find(starting_pin_types.begin(), starting_pin_types.end(), ep->get_pin()->get_type()) != starting_pin_types.end())
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }

                    if (std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) != forbidden_pin_types.end())
                    {
                        return false;
                    }

                    return true;
                };

                const auto endpoint_filter_b = [g_b, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g_b && !starting_pin_types.empty())
                    {
                        if (std::find(starting_pin_types.begin(), starting_pin_types.end(), ep->get_pin()->get_type()) != starting_pin_types.end())
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }

                    if (std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) != forbidden_pin_types.end())
                    {
                        return false;
                    }

                    return true;
                };

                const auto neighborhood_a = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                                .get_next_matching_gates_until(g_a, [](const auto* g) { return true; }, m_direction, m_directed, false, endpoint_filter_a, endpoint_filter_a);

                const auto neighborhood_b = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                                .get_next_matching_gates_until(g_b, [](const auto* g) { return true; }, m_direction, m_directed, false, endpoint_filter_b, endpoint_filter_b);

                if (neighborhood_a.is_error())
                {
                    return ERR_APPEND(neighborhood_a.get_error(), "cannot calculate feature " + to_string());
                }

                if (neighborhood_b.is_error())
                {
                    return ERR_APPEND(neighborhood_b.get_error(), "cannot calculate feature " + to_string());
                }

                std::set<const Gate*> neighbors_a = g_a->get_type()->has_property(GateTypeProperty::sequential) ? std::set<const Gate*>{g_a} : std::set<const Gate*>{};
                std::set<const Gate*> neighbors_b = g_b->get_type()->has_property(GateTypeProperty::sequential) ? std::set<const Gate*>{g_b} : std::set<const Gate*>{};

                for (const auto g_n : neighborhood_a.get())
                {
                    neighbors_a.insert(g_n);
                }

                for (const auto g_n : neighborhood_b.get())
                {
                    neighbors_b.insert(g_n);
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
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                // fix to make compiler happpy
                const auto depth               = m_depth;
                const auto forbidden_pin_types = m_forbidden_pin_types;
                const auto starting_pin_types  = m_starting_pin_types;

                const auto endpoint_filter_a = [g_a, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g_a && !starting_pin_types.empty())
                    {
                        if (std::find(starting_pin_types.begin(), starting_pin_types.end(), ep->get_pin()->get_type()) != starting_pin_types.end())
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }

                    if (std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) != forbidden_pin_types.end())
                    {
                        return false;
                    }

                    return true;
                };

                const auto endpoint_filter_b = [g_b, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g_b && !starting_pin_types.empty())
                    {
                        if (std::find(starting_pin_types.begin(), starting_pin_types.end(), ep->get_pin()->get_type()) != starting_pin_types.end())
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }

                    if (std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) != forbidden_pin_types.end())
                    {
                        return false;
                    }

                    return true;
                };

                const auto neighborhood_a = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                                .get_next_matching_gates_until(g_a, [](const auto* g) { return true; }, m_direction, m_directed, false, endpoint_filter_a, endpoint_filter_a);

                const auto neighborhood_b = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                                .get_next_matching_gates_until(g_b, [](const auto* g) { return true; }, m_direction, m_directed, false, endpoint_filter_b, endpoint_filter_b);

                if (neighborhood_a.is_error())
                {
                    return ERR_APPEND(neighborhood_a.get_error(), "cannot calculate feature " + to_string());
                }

                if (neighborhood_b.is_error())
                {
                    return ERR_APPEND(neighborhood_b.get_error(), "cannot calculate feature " + to_string());
                }

                std::set<const Gate*> neighbors_a = {g_a};
                std::set<const Gate*> neighbors_b = {g_b};

                for (const auto g_n : neighborhood_a.get())
                {
                    neighbors_a.insert(g_n);
                }

                for (const auto g_n : neighborhood_b.get())
                {
                    neighbors_b.insert(g_n);
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