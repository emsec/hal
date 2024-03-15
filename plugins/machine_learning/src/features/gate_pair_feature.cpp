#include "machine_learning/features/gate_pair_feature.h"

#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace machine_learning
    {
        namespace gate_pair_feature
        {
            std::vector<u32> LogicalDistance::calculate_feature(FeatureContext& fc, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return {0};
                }

                const auto shortest_path = NetlistTraversalDecorator(*fc.nl).get_shortest_path(g_a, g_b, true);

                if (shortest_path.empty())
                {
                    return {64};
                }

                if (shortest_path.size() <= 1)
                {
                    log_error("machine_learning", "Found shortest path with only one component, this is unexpected!");
                }

                const u32 distance = shortest_path.size() - 2;
                return {std::min(distance, u32(64))};
            };

            std::string LogicalDistance::get_name() const
            {
                return "LogicalDistance";
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
                std::set<const Gate*> neighbors_a;
                std::set<const Gate*> neighbors_b;

                if (m_direction == GraphDirection::directed_forward)
                {
                    std::vector<const Gate*> q = {g_a};

                    for (const auto g_n : NetlistTraversalDecorator(*fc.nl).get_next_sequential_gates(g_a, true, m_depth))
                    {
                        neighbors_a.insert(g_n);
                    }

                    for (const auto g_n : NetlistTraversalDecorator(*fc.nl).get_next_sequential_gates(g_b, true, m_depth))
                    {
                        neighbors_b.insert(g_n);
                    }
                }

                if (m_direction == GraphDirection::directed_backward)
                {
                    std::vector<const Gate*> q = {g_a};

                    for (const auto g_n : NetlistTraversalDecorator(*fc.nl).get_next_sequential_gates(g_a, false, m_depth))
                    {
                        neighbors_a.insert(g_n);
                    }

                    for (const auto g_n : NetlistTraversalDecorator(*fc.nl).get_next_sequential_gates(g_b, false, m_depth))
                    {
                        neighbors_b.insert(g_n);
                    }
                }

                if (m_direction == GraphDirection::bidirectional)
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
                std::set<const Gate*> neighbors_a;
                std::set<const Gate*> neighbors_b;

                if (m_direction == GraphDirection::directed_forward || m_direction == GraphDirection::directed_backward)
                {
                    std::vector<const Gate*> q = {g_a};

                    const bool search_successors = (m_direction == directed_forward);

                    auto subgraph_a_res =
                        NetlistTraversalDecorator(*fc.nl).get_subgraph_gates(g_a, search_successors, nullptr, [this](const auto& _ep, const auto& cur_depth) { return cur_depth <= this->m_depth; });
                    auto subgraph_b_res =
                        NetlistTraversalDecorator(*fc.nl).get_subgraph_gates(g_b, search_successors, nullptr, [this](const auto& _ep, const auto& cur_depth) { return cur_depth <= this->m_depth; });

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
                }

                if (m_direction == GraphDirection::bidirectional)
                {
                    // NOTE this is either trivial by just combining both directions or more complex by building a real undirected graph (predecessors of a successor would also be neighbors eventhough the direction siwtches)
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

            std::vector<std::vector<u32>> test_build_feature_vec(const Netlist* nl, const std::vector<std::pair<Gate*, Gate*>>& pairs)
            {
                const auto t0 = LogicalDistance();
                const auto t1 = SharedControlSignals();
                const auto t2 = SharedSequentialNeighbors(1, GraphDirection::directed_backward);
                const auto t3 = SharedSequentialNeighbors(1, GraphDirection::directed_forward);
                const auto t4 = SharedSequentialNeighbors(2, GraphDirection::directed_backward);
                const auto t5 = SharedSequentialNeighbors(2, GraphDirection::directed_forward);
                const auto t6 = SharedNeighbors(1, GraphDirection::directed_backward);
                const auto t7 = SharedNeighbors(1, GraphDirection::directed_forward);
                const auto t8 = SharedNeighbors(2, GraphDirection::directed_backward);
                const auto t9 = SharedNeighbors(2, GraphDirection::directed_forward);
                // const auto tx = PhysicalDistance();

                const std::vector<const GatePairFeature*> features = {
                    // dynamic_cast<const GatePairFeature*>(&t0),
                    dynamic_cast<const GatePairFeature*>(&t1),
                    //   dynamic_cast<const GatePairFeature*>(&t2),
                    //   dynamic_cast<const GatePairFeature*>(&t3),
                    //   dynamic_cast<const GatePairFeature*>(&t4),
                    //   dynamic_cast<const GatePairFeature*>(&t5),
                    //   dynamic_cast<const GatePairFeature*>(&t6),
                    //   dynamic_cast<const GatePairFeature*>(&t7),
                    //   dynamic_cast<const GatePairFeature*>(&t8),
                    //   dynamic_cast<const GatePairFeature*>(&t9)
                };

                FeatureContext fc = FeatureContext{nl};
                std::vector<std::vector<u32>> pair_features;

                for (const auto& [g_a, g_b] : pairs)
                {
                    pair_features.push_back(build_feature_vec(fc, features, g_a, g_b));
                }

                return pair_features;
            }
        }    // namespace gate_pair_feature
    }        // namespace machine_learning
}    // namespace hal