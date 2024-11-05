#include "machine_learning/features/gate_pair_feature.h"

#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#define MAX_DISTANCE 255

namespace hal
{
    namespace machine_learning
    {
        namespace gate_pair_feature
        {
            const NetlistAbstraction& FeatureContext::get_sequential_abstraction()
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

                    m_seqential_abstraction = NetlistAbstraction(nl, seq_gates, true, endpoint_filter, endpoint_filter);
                }

                return m_seqential_abstraction.value();
            }

            std::vector<u32> LogicalDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return {0};
                }

                const auto res = NetlistTraversalDecorator(*fc.nl).get_shortest_path_distance(g_a, g_b, m_direction);
                if (res.is_error())
                {
                    log_error("machine_learning", "failed to calculate shortest path feature: {}", res.get_error().get());
                    return {};
                }

                const auto shortest_path = res.get();

                if (!shortest_path.has_value())
                {
                    return {MAX_DISTANCE};
                }

                if (shortest_path.value() < 1)
                {
                    log_error("machine_learning", "Found shortest path with no components, this is unexpected!");
                }

                const u32 distance = shortest_path.value();
                return {std::min(distance, u32(MAX_DISTANCE))};
            };

            std::string LogicalDistance::get_name() const
            {
                return "LogicalDistance";
            }

            std::vector<u32> SequentialDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return {0};
                }

                const auto res = NetlistAbstractionDecorator(fc.get_sequential_abstraction()).get_shortest_path_distance(g_a, g_b, m_direction);
                if (res.is_error())
                {
                    log_error("machine_learning", "failed to calculate shortest path feature: {}", res.get_error().get());
                    return {};
                }

                const auto shortest_path = res.get();

                if (!shortest_path.has_value())
                {
                    return {MAX_DISTANCE};
                }

                if (shortest_path.value() <= 1)
                {
                    log_error("machine_learning", "Found shortest path with only one component, this is unexpected!");
                }

                const u32 distance = shortest_path.value();
                return {std::min(distance, u32(MAX_DISTANCE))};
            };

            std::string SequentialDistance::get_name() const
            {
                return "SequentialDistance";
            }

            std::vector<u32> PhysicalDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                log_error("machine_learning", "Physical distance currently not implemented as gate pair feature.");

                return {};
            };

            std::string PhysicalDistance::get_name() const
            {
                return "PhysicalDistance";
            }

            std::vector<u32> SharedControlSignals::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
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

                return {u32(shared.size())};
            };

            std::string SharedControlSignals::get_name() const
            {
                return "SharedControlSignals";
            }

            std::vector<u32> SharedSequentialNeighbors::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
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
                        log_error("machine_learning", "cannot calculate feature {}: ", this->get_name());
                        return {};
                    }

                    for (const auto g_n : res_a.get())
                    {
                        neighbors_a.insert(g_n);
                    }

                    const auto res_b = get_n_next_sequential_gates(g_b, true, m_depth);
                    if (res_b.is_error())
                    {
                        log_error("machine_learning", "cannot calculate feature {}: ", this->get_name());
                        return {};
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
                        log_error("machine_learning", "cannot calculate feature {}: ", this->get_name());
                        return {};
                    }

                    for (const auto g_n : res_a.get())
                    {
                        neighbors_a.insert(g_n);
                    }

                    const auto res_b = get_n_next_sequential_gates(g_b, false, m_depth);
                    if (res_b.is_error())
                    {
                        log_error("machine_learning", "cannot calculate feature {}: ", this->get_name());
                        return {};
                    }

                    for (const auto g_n : res_b.get())
                    {
                        neighbors_b.insert(g_n);
                    }
                }

                if (m_direction == PinDirection::inout)
                {
                    // NOTE this is either trivial by just combining both directions or more complex by building a real undirected graph (predecessors of a successor would also be neighbors eventhough the direction siwtches)
                    log_error("machine_learning", "SharedSequentialNeighbors bidirectional feature not implemented yet");
                }

                std::set<const Gate*> shared;
                std::set_intersection(neighbors_a.begin(), neighbors_a.end(), neighbors_b.begin(), neighbors_b.end(), std::inserter(shared, shared.begin()));

                return {u32(shared.size())};
            };

            std::string SharedSequentialNeighbors::get_name() const
            {
                return "SharedSequentialNeighbors";
            }

            std::vector<u32> SharedNeighbors::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
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
                        log_error(
                            "machine_learning", "cannot calculate shared neighbors feature: failed to build subgraph of depth {} for gate {} with ID {}", m_depth, g_a->get_name(), g_a->get_id());
                        return {};
                    }

                    if (subgraph_b_res.is_error())
                    {
                        log_error(
                            "machine_learning", "cannot calculate shared neighbors feature: failed to build subgraph of depth {} for gate {} with ID {}", m_depth, g_b->get_name(), g_b->get_id());
                        return {};
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
                    std::cout << "Subgraph A: " << std::endl;
                    for (const auto& g_a : neighbors_a)
                    {
                        std::cout << "\t" << g_a->get_id() << " / " << g_a->get_name() << std::endl;
                    }

                    std::cout << "Subgraph B: " << std::endl;
                    for (const auto& g_b : neighbors_b)
                    {
                        std::cout << "\t" << g_b->get_id() << " / " << g_b->get_name() << std::endl;
                    }
                }

                if (m_direction == PinDirection::inout)
                {
                    // NOTE this is either trivial by just combining both directions or more complex by building a real undirected graph (predecessors of a successor would also be neighbors eventhough the direction switches)
                    log_error("machine_learning", "SharedNeighbors bidirectional feature not implemented yet");
                }

                std::set<const Gate*> shared;
                std::set_intersection(neighbors_a.begin(), neighbors_a.end(), neighbors_b.begin(), neighbors_b.end(), std::inserter(shared, shared.begin()));

                return {u32(shared.size())};
            };

            std::string SharedNeighbors::get_name() const
            {
                return "SharedNeighbors";
            }

            std::vector<u32> build_feature_vec(const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
            {
                FeatureContext fc(g_a->get_netlist());
                return build_feature_vec(fc, features, g_a, g_b);
            }

            std::vector<u32> build_feature_vec(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
            {
                std::vector<u32> feature_vec;

                for (const auto& gf : features)
                {
                    // TODO remove
                    // std::cout << "Calculating feature " << gf->get_name() << " for g_a " << g_a->get_id() << " and g_b " << g_b->get_id() << std::endl;

                    const auto new_features = gf->calculate_feature(fc, g_a, g_b);
                    feature_vec.insert(feature_vec.end(), new_features.begin(), new_features.end());
                }

                return feature_vec;
            }

            std::vector<u32> build_feature_vec(const std::vector<const GatePairFeature*>& features, const std::pair<const Gate*, const Gate*>& gate_pair)
            {
                return build_feature_vec(features, gate_pair.first, gate_pair.second);
            }

            std::vector<u32> build_feature_vec(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const std::pair<const Gate*, const Gate*>& gate_pair)
            {
                return build_feature_vec(fc, features, gate_pair.first, gate_pair.second);
            }

            std::vector<std::vector<u32>> build_feature_vecs(const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<const Gate*, const Gate*>>& gate_pairs)
            {
                if (gate_pairs.empty())
                {
                    return {};
                }

                FeatureContext fc(gate_pairs.front().first->get_netlist());
                return build_feature_vecs(fc, features, gate_pairs);
            }

            std::vector<std::vector<u32>>
                build_feature_vecs(FeatureContext& fc, const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<const Gate*, const Gate*>>& gate_pairs)
            {
                std::vector<std::vector<u32>> feature_vecs;

                for (const auto& gp : gate_pairs)
                {
                    feature_vecs.push_back(build_feature_vec(fc, features, gp));
                }

                return feature_vecs;
            }
        }    // namespace gate_pair_feature
    }    // namespace machine_learning
}    // namespace hal