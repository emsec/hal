#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "machine_learning/features/gate_feature.h"

#include <vector>

#define MAX_DISTANCE FEATURE_TYPE(255)
// #define PROGRESS_BAR

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
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

                                                  // TODO remove debug print
                                                  //   if (!global_io_connections.get().empty())
                                                  //   {
                                                  //       std::cout << "Global IO connections: " << global_io_connections.get().front()->get_name() << std::endl;
                                                  //       std::cout << "Endpoint: " << ep->get_pin()->get_name() << std::endl;
                                                  //   }

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
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal