#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/progress_printer.h"
#include "machine_learning/features/gate_pair_feature.h"

#define MAX_DISTANCE FEATURE_TYPE(255)
// #define PROGRESS_BAR

namespace hal
{
    namespace machine_learning
    {
        namespace gate_pair_feature
        {
            Result<std::vector<FEATURE_TYPE>> LogicalDistance::calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return OK({FEATURE_TYPE(0)});
                }

                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_original_abstraction();
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

                const auto res_ab = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_a, g_b, m_direction, m_directed, endpoint_filter, endpoint_filter);
                if (res_ab.is_error())
                {
                    return ERR_APPEND(res_ab.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                }

                const auto shortest_path_ab = res_ab.get();

                if (!shortest_path_ab.has_value())
                {
                    return OK({MAX_DISTANCE});
                }

                if (shortest_path_ab.value() < 1)
                {
                    log_error("machine_learning", "Found shortest path with no components, this is unexpected!");
                }

                const u32 distance_ab = shortest_path_ab.value();

                if (m_direction != PinDirection::inout)
                {
                    const auto res_ba = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_b, g_a, m_direction, m_directed, endpoint_filter, endpoint_filter);
                    if (res_ba.is_error())
                    {
                        return ERR_APPEND(res_ba.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                    }

                    const auto shortest_path_ba = res_ba.get();

                    if (!shortest_path_ba.has_value())
                    {
                        return OK({MAX_DISTANCE});
                    }

                    if (shortest_path_ba.value() < 1)
                    {
                        log_error("machine_learning", "Found shortest path with only one component, this is unexpected!");
                    }

                    const u32 distance_ba = shortest_path_ba.value();
                    return OK({FEATURE_TYPE(std::min(distance_ab, u32(MAX_DISTANCE))), FEATURE_TYPE(std::min(distance_ba, u32(MAX_DISTANCE)))});
                }

                return OK({FEATURE_TYPE(std::min(distance_ab, u32(MAX_DISTANCE)))});
            };

            std::string LogicalDistance::to_string() const
            {
                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "LogicalDistance_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            Result<std::vector<FEATURE_TYPE>> SequentialDistance::calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                if (g_a == g_b)
                {
                    return OK({FEATURE_TYPE(0)});
                }

                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_sequential_abstraction();
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

                const auto res_ab = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_a, g_b, m_direction, m_directed, endpoint_filter, endpoint_filter);
                if (res_ab.is_error())
                {
                    return ERR_APPEND(res_ab.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                }

                const auto shortest_path_ab = res_ab.get();

                if (!shortest_path_ab.has_value())
                {
                    return OK({MAX_DISTANCE});
                }

                if (shortest_path_ab.value() < 1)
                {
                    log_error("machine_learning", "Found shortest path with only one component, this is unexpected!");
                }

                const u32 distance_ab = shortest_path_ab.value();

                if (m_direction != PinDirection::inout)
                {
                    const auto res_ba = NetlistAbstractionDecorator(*(nl_abstr.get())).get_shortest_path_distance(g_b, g_a, m_direction, m_directed, endpoint_filter, endpoint_filter);
                    if (res_ba.is_error())
                    {
                        return ERR_APPEND(res_ba.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                    }

                    const auto shortest_path_ba = res_ba.get();

                    if (!shortest_path_ba.has_value())
                    {
                        return OK({MAX_DISTANCE});
                    }

                    if (shortest_path_ba.value() < 1)
                    {
                        log_error("machine_learning", "Found shortest path with only one component, this is unexpected!");
                    }

                    const u32 distance_ba = shortest_path_ba.value();
                    return OK({FEATURE_TYPE(std::min(distance_ab, u32(MAX_DISTANCE))), FEATURE_TYPE(std::min(distance_ba, u32(MAX_DISTANCE)))});
                }

                return OK({FEATURE_TYPE(std::min(distance_ab, u32(MAX_DISTANCE)))});
            };

            std::string SequentialDistance::to_string() const
            {
                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "SequentialDistance_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            Result<std::vector<FEATURE_TYPE>> PhysicalDistance::calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                UNUSED(ctx);
                UNUSED(g_a);
                UNUSED(g_b);
                log_error("machine_learning", "Physical distance currently not implemented as gate pair feature.");

                return OK({});
            };

            std::string PhysicalDistance::to_string() const
            {
                return "PhysicalDistance";
            }

            Result<std::vector<FEATURE_TYPE>> SharedControlSignals::calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const
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

                return OK({FEATURE_TYPE(shared.size())});
            };

            std::string SharedControlSignals::to_string() const
            {
                return "SharedControlSignals";
            }

            Result<std::vector<FEATURE_TYPE>> SharedSequentialNeighbors::calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_sequential_abstraction();
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
                                                .get_next_matching_gates_until(
                                                    g_a,
                                                    [](const auto* _g) {
                                                        UNUSED(_g);
                                                        return true;
                                                    },
                                                    m_direction,
                                                    m_directed,
                                                    false,
                                                    endpoint_filter_a,
                                                    endpoint_filter_a);

                const auto neighborhood_b = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                                .get_next_matching_gates_until(
                                                    g_b,
                                                    [](const auto* _g) {
                                                        UNUSED(_g);
                                                        return true;
                                                    },
                                                    m_direction,
                                                    m_directed,
                                                    false,
                                                    endpoint_filter_b,
                                                    endpoint_filter_b);

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

                return OK({FEATURE_TYPE(shared.size())});
            };

            std::string SharedSequentialNeighbors::to_string() const
            {
                std::string starting_pin_types_str = utils::join("_", m_starting_pin_types.begin(), m_starting_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "SharedSequentialNeighbors_" + std::to_string(m_depth) + "_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_"
                       + (starting_pin_types_str.empty() ? "None" : starting_pin_types_str) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            Result<std::vector<FEATURE_TYPE>> SharedNeighbors::calculate_feature(Context& ctx, const Gate* g_a, const Gate* g_b) const
            {
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_original_abstraction();
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
                                                .get_next_matching_gates_until(
                                                    g_a,
                                                    [](const auto* _g) {
                                                        UNUSED(_g);
                                                        return true;
                                                    },
                                                    m_direction,
                                                    m_directed,
                                                    false,
                                                    endpoint_filter_a,
                                                    endpoint_filter_a);

                const auto neighborhood_b = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                                .get_next_matching_gates_until(
                                                    g_b,
                                                    [](const auto* _g) {
                                                        UNUSED(_g);
                                                        return true;
                                                    },
                                                    m_direction,
                                                    m_directed,
                                                    false,
                                                    endpoint_filter_b,
                                                    endpoint_filter_b);

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

                return OK({FEATURE_TYPE(shared.size())});
            };

            std::string SharedNeighbors::to_string() const
            {
                std::string starting_pin_types_str = utils::join("_", m_starting_pin_types.begin(), m_starting_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "SharedNeighbors_" + std::to_string(m_depth) + "_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_"
                       + (starting_pin_types_str.empty() ? "None" : starting_pin_types_str) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
            {
                Context ctx(g_a->get_netlist());
                return build_feature_vec(ctx, features, g_a, g_b);
            }

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const GatePairFeature*>& features, const Gate* g_a, const Gate* g_b)
            {
                std::vector<FEATURE_TYPE> feature_vec;

                for (const auto& gf : features)
                {
                    // TODO remove
                    //std::cout << "Calculating feature " << gf->to_string() << " for g_a " << g_a->get_id() << " and g_b " << g_b->get_id() << std::endl;

                    const auto new_features = gf->calculate_feature(ctx, g_a, g_b);
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

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(const std::vector<const GatePairFeature*>& features, const std::pair<Gate*, Gate*>& gate_pair)
            {
                return build_feature_vec(features, gate_pair.first, gate_pair.second);
            }

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const GatePairFeature*>& features, const std::pair<Gate*, Gate*>& gate_pair)
            {
                return build_feature_vec(ctx, features, gate_pair.first, gate_pair.second);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs)
            {
                if (gate_pairs.empty())
                {
                    return OK({});
                }

                Context ctx(gate_pairs.front().first->get_netlist());
                return build_feature_vecs(ctx, features, gate_pairs);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(Context& ctx, const std::vector<const GatePairFeature*>& features, const std::vector<std::pair<Gate*, Gate*>>& gate_pairs)
            {
                // Preallocate the feature vectors
                std::vector<std::vector<FEATURE_TYPE>> feature_vecs(gate_pairs.size());
                std::vector<Result<std::monostate>> thread_results(ctx.num_threads, ERR("uninitialized"));

#ifdef PROGRESS_BAR
                const auto msg = "Calculated gate pair features for " + std::to_string(gate_pairs.size()) + "/" + std::to_string(gate_pairs.size()) + " pairs";
                auto pp        = ProgressPrinter(msg.size());
                u32 pp_counter = 0;
                std::mutex pp_mutex;
#endif

                // Worker function for each thread
                auto thread_func = [&](u32 start, u32 end, u32 thread_index) {
                    for (u32 i = start; i < end; ++i)
                    {
                        const auto& gp         = gate_pairs[i];
                        const auto feature_vec = build_feature_vec(ctx, features, gp);
                        if (feature_vec.is_error())
                        {
                            thread_results.at(thread_index) =
                                ERR_APPEND(feature_vec.get_error(),
                                           "cannot build feature vecs: failed to build feature vector for gate_a " + gp.first->get_name() + " with ID " + std::to_string(gp.first->get_id())
                                               + " and gate_b " + gp.second->get_name() + " with ID " + std::to_string(gp.second->get_id()));
                            return;
                        }

                        feature_vecs[i] = feature_vec.get();

#ifdef PROGRESS_BAR
                        {
                            std::lock_guard<std::mutex> lock(pp_mutex);
                            pp_counter++;
                            pp.print_progress(static_cast<float>(pp_counter) / gate_pairs.size(),
                                              "Calculated gate pair features for " + std::to_string(pp_counter) + "/" + std::to_string(gate_pairs.size()) + " pairs");
                        }
#endif
                    }

                    thread_results.at(thread_index) = OK({});
                };

                // Launch threads to process gate_pairs in parallel
                std::vector<std::thread> threads;
                u32 chunk_size = (gate_pairs.size() + ctx.num_threads - 1) / ctx.num_threads;
                for (u32 t = 0; t < ctx.num_threads; ++t)
                {
                    u32 start = t * chunk_size;
                    u32 end   = std::min(start + chunk_size, static_cast<u32>(gate_pairs.size()));
                    if (start < end)
                    {
                        threads.emplace_back(thread_func, start, end, t);
                    }
                }

                for (auto& thread : threads)
                {
                    thread.join();
                }

                // Check whether a thread encountered an error
                for (const auto& res : thread_results)
                {
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "Encountered error when building feature vectors");
                    }
                }

#ifdef PROGRESS_BAR
                pp.clear();
#endif

                return OK(feature_vecs);
            }

        }    // namespace gate_pair_feature
    }        // namespace machine_learning
}    // namespace hal