#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/progress_printer.h"
#include "machine_learning/features/gate_feature_single.h"

#include <vector>

#define MAX_DISTANCE FEATURE_TYPE(255)
// #define PROGRESS_BAR

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            Result<std::vector<FEATURE_TYPE>> ConnectedGlobalIOs::calculate_feature(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                u32 connected_global_inputs  = 0;
                u32 connected_global_outputs = 0;

                for (const auto& in : g->get_fan_in_nets())
                {
                    if (in->is_global_input_net())
                    {
                        connected_global_inputs += 1;
                    }
                }

                for (const auto& out : g->get_fan_out_nets())
                {
                    if (out->is_global_output_net())
                    {
                        connected_global_outputs += 1;
                    }
                }

                return OK({FEATURE_TYPE(connected_global_inputs), FEATURE_TYPE(connected_global_outputs)});
            }

            std::string ConnectedGlobalIOs::to_string() const
            {
                return "ConnectedGlobalIOs";
            }

            Result<std::vector<FEATURE_TYPE>> DistanceGlobalIO::calculate_feature(Context& ctx, const Gate* g) const
            {
                // necessary workaround to please compiler
                const auto& direction                                = m_direction;
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                // necessary workaround to please compiler
                const auto& forbidden_pin_types = m_forbidden_pin_types;
                const auto endpoint_filter      = [forbidden_pin_types](const auto* ep, const auto& _d) {
                    UNUSED(_d);
                    return std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) == forbidden_pin_types.end();
                };

                const auto distance = NetlistAbstractionDecorator(*nl_abstr.get())
                                          .get_shortest_path_distance(
                                              g,
                                              [direction](const auto* ep, const auto& nla) {
                                                  if (!((direction == PinDirection::output) ? ep->is_source_pin() : ep->is_destination_pin()))
                                                  {
                                                      return false;
                                                  }

                                                  const auto global_io_connections = (direction == PinDirection::output) ? nla.get_global_output_successors(ep) : nla.get_global_input_predecessors(ep);
                                                  if (global_io_connections.is_error())
                                                  {
                                                      log_error("machine_learning", "{}", global_io_connections.get_error().get());
                                                      return false;
                                                  }
                                                  return !global_io_connections.get().empty();
                                              },
                                              m_direction,
                                              m_directed,
                                              endpoint_filter,
                                              endpoint_filter);

                if (distance.is_error())
                {
                    return ERR_APPEND(distance.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                }

                if (!distance.get().has_value())
                {
                    return OK({MAX_DISTANCE});
                }

                const auto feature = std::min(distance.get().value(), u32(MAX_DISTANCE));

                return OK({FEATURE_TYPE(feature)});
            }

            std::string DistanceGlobalIO::to_string() const
            {
                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "DistanceGlobalIO_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            Result<std::vector<FEATURE_TYPE>> SequentialDistanceGlobalIO::calculate_feature(Context& ctx, const Gate* g) const
            {
                // necessary workaround to please compiler
                const auto& direction                                = m_direction;
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get sequential netlist abstraction");
                }

                // necessary workaround to please compiler
                const auto& forbidden_pin_types = m_forbidden_pin_types;
                const auto endpoint_filter      = [forbidden_pin_types](const auto* ep, const auto& _d) {
                    UNUSED(_d);
                    return std::find(forbidden_pin_types.begin(), forbidden_pin_types.end(), ep->get_pin()->get_type()) == forbidden_pin_types.end();
                };

                const auto distance = NetlistAbstractionDecorator(*nl_abstr.get())
                                          .get_shortest_path_distance(
                                              g,
                                              [direction](const auto* ep, const auto& nla) {
                                                  if (!((direction == PinDirection::output) ? ep->is_source_pin() : ep->is_destination_pin()))
                                                  {
                                                      return false;
                                                  }

                                                  const auto global_io_connections = (direction == PinDirection::output) ? nla.get_global_output_successors(ep) : nla.get_global_input_predecessors(ep);
                                                  if (global_io_connections.is_error())
                                                  {
                                                      log_error("machine_learning", "{}", global_io_connections.get_error().get());
                                                      return false;
                                                  }
                                                  return !global_io_connections.get().empty();
                                              },
                                              m_direction,
                                              m_directed,
                                              endpoint_filter,
                                              endpoint_filter);

                if (distance.is_error())
                {
                    return ERR_APPEND(distance.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate shortest path distance.");
                }

                if (!distance.get().has_value())
                {
                    return OK({MAX_DISTANCE});
                }

                const auto feature = std::min(distance.get().value(), u32(MAX_DISTANCE));

                return OK({FEATURE_TYPE(feature)});
            }

            std::string SequentialDistanceGlobalIO::to_string() const
            {
                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "SequentialDistanceGlobalIO_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            Result<std::vector<FEATURE_TYPE>> IODegrees::calculate_feature(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                u32 input_io_degree  = g->get_fan_in_nets().size();
                u32 output_io_degree = g->get_fan_out_nets().size();

                return OK({FEATURE_TYPE(input_io_degree), FEATURE_TYPE(output_io_degree)});
            }

            std::string IODegrees::to_string() const
            {
                return "IODegrees";
            }

            Result<std::vector<FEATURE_TYPE>> GateTypeOneHot::calculate_feature(Context& ctx, const Gate* g) const
            {
                const auto& all_properties = ctx.get_possible_gate_type_properties();

                // TODO remove debug print
                // std::cout << "Got following gate type properties: " << std::endl;
                // for (const auto& gtp : all_properties)
                // {
                //     std::cout << enum_to_string(gtp) << std::endl;
                // }

                std::vector<FEATURE_TYPE> feature = std::vector<FEATURE_TYPE>(all_properties.size(), FEATURE_TYPE(0));

                for (const auto& gtp : g->get_type()->get_properties())
                {
                    const u32 index = std::distance(all_properties.begin(), std::find(all_properties.begin(), all_properties.end(), gtp));
                    feature.at(index) += FEATURE_TYPE(1);
                }

                return OK(feature);
            }

            std::string GateTypeOneHot::to_string() const
            {
                return "GateTypeOneHot";
            }

            Result<std::vector<FEATURE_TYPE>> NeighboringGateTypes::calculate_feature(Context& ctx, const Gate* g) const
            {
                const auto& all_properties = ctx.get_possible_gate_type_properties();

                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                std::vector<FEATURE_TYPE> feature = std::vector<FEATURE_TYPE>(all_properties.size(), FEATURE_TYPE(0));

                // fix to make compiler happpy
                const auto depth = m_depth;

                const auto neighborhood = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                              .get_next_matching_gates_until(
                                                  g,
                                                  [](const auto* _g) {
                                                      UNUSED(_g);
                                                      return true;
                                                  },
                                                  m_direction,
                                                  m_directed,
                                                  false,
                                                  [depth](const auto* _ep, const auto current_depth) {
                                                      UNUSED(_ep);
                                                      return current_depth <= depth;
                                                  },
                                                  nullptr);

                if (neighborhood.is_error())
                {
                    return ERR_APPEND(neighborhood.get_error(), "cannot calculate feature " + to_string());
                }

                for (const auto& gn : neighborhood.get())
                {
                    for (const auto& gtp : gn->get_type()->get_properties())
                    {
                        const u32 index = std::distance(all_properties.begin(), std::find(all_properties.begin(), all_properties.end(), gtp));
                        feature.at(index) += FEATURE_TYPE(1);
                    }
                }

                return OK(feature);
            }

            std::string NeighboringGateTypes::to_string() const
            {
                return "NeighboringGateTypes_" + std::to_string(m_depth) + "_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> GateFeatureSingle::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                // Preallocate the feature vectors
                std::vector<std::vector<FEATURE_TYPE>> feature_vecs(gates.size());
                std::vector<Result<std::monostate>> thread_results(ctx.num_threads, ERR("uninitialized"));

                // Worker function for each thread
                auto thread_func = [&](u32 start, u32 end, u32 thread_index) {
                    for (u32 i = start; i < end; ++i)
                    {
                        const auto feature_vec = (calculate_feature(ctx, gates.at(i)));
                        if (feature_vec.is_error())
                        {
                            thread_results.at(thread_index) = ERR(feature_vec.get_error().get());
                            return;
                        }

                        feature_vecs[i] = feature_vec.get();
                    }

                    thread_results.at(thread_index) = OK({});
                    return;
                };

                // Launch threads to process gates in parallel
                std::vector<std::thread> threads;
                u32 chunk_size = (gates.size() + ctx.num_threads - 1) / ctx.num_threads;
                for (u32 t = 0; t < ctx.num_threads; ++t)
                {
                    u32 start = t * chunk_size;
                    u32 end   = std::min(start + chunk_size, u32(gates.size()));
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

                return OK(feature_vecs);
            }
        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal