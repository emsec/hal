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

            std::vector<std::string> ConnectedGlobalIOs::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"connected_global_inputs", "connected_global_outputs"};
            }

            Result<std::vector<FEATURE_TYPE>> PinCount::calculate_feature(Context& ctx, const Gate* g) const
            {
                UNUSED(ctx);

                const auto& directions = m_directions;
                const auto& pin_types  = m_pin_types;

                const auto matching_pins = g->get_type()->get_pins([&directions, &pin_types](const GatePin* p) {
                    if (!directions.empty() && std::find(directions.begin(), directions.end(), p->get_direction()) == directions.end())
                    {
                        return false;
                    }
                    if (!pin_types.empty() && std::find(pin_types.begin(), pin_types.end(), p->get_type()) == pin_types.end())
                    {
                        return false;
                    }
                    return true;
                });

                return OK({FEATURE_TYPE(matching_pins.size())});
            }

            std::string PinCount::to_string() const
            {
                const std::string directions_str = utils::join("_", m_directions.begin(), m_directions.end(), [](const PinDirection& d) { return enum_to_string(d); });
                const std::string pin_types_str  = utils::join("_", m_pin_types.begin(), m_pin_types.end(), [](const PinType& t) { return enum_to_string(t); });

                return "PinCount_" + (directions_str.empty() ? "None" : directions_str) + "_" + (pin_types_str.empty() ? "None" : pin_types_str);
            }

            std::vector<std::string> PinCount::get_legend(Context& ctx) const
            {
                UNUSED(ctx);

                const std::string directions_str = m_directions.empty()
                                                       ? "any_direction"
                                                       : utils::join("_or_", m_directions.begin(), m_directions.end(), [](const PinDirection& d) { return enum_to_string(d); });
                const std::string pin_types_str =
                    m_pin_types.empty() ? "any_type" : utils::join("_or_", m_pin_types.begin(), m_pin_types.end(), [](const PinType& t) { return enum_to_string(t); });

                return {"pin_count_" + directions_str + "_" + pin_types_str};
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

            std::vector<std::string> GateTypeOneHot::get_legend(Context& ctx) const
            {
                const auto& all_properties = ctx.get_possible_gate_type_properties();

                std::vector<std::string> legend;
                legend.reserve(all_properties.size());
                for (const auto& gtp : all_properties)
                {
                    legend.push_back("property_" + enum_to_string(gtp));
                }
                return legend;
            }

            Result<std::vector<FEATURE_TYPE>> NeighborCount::calculate_feature(Context& ctx, const Gate* g) const
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

                const auto endpoint_filter = [g, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g && !starting_pin_types.empty())
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
                                                  endpoint_filter,
                                                  endpoint_filter);

                if (neighborhood.is_error())
                {
                    return ERR_APPEND(neighborhood.get_error(), "cannot calculate feature " + to_string());
                }

                const std::vector<FEATURE_TYPE> feature = {FEATURE_TYPE(neighborhood.get().size())};

                return OK(feature);
            }

            std::string NeighborCount::to_string() const
            {
                std::string starting_pin_types_str = utils::join("_", m_starting_pin_types.begin(), m_starting_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "NeighborCount_" + std::to_string(m_depth) + "_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_"
                       + (starting_pin_types_str.empty() ? "None" : starting_pin_types_str) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            std::vector<std::string> NeighborCount::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"neighbor_count"};
            }

            Result<std::vector<FEATURE_TYPE>> SequentialNeighborCount::calculate_feature(Context& ctx, const Gate* g) const
            {
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = ctx.get_sequential_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                // fix to make compiler happpy
                const auto depth               = m_depth;
                const auto forbidden_pin_types = m_forbidden_pin_types;
                const auto starting_pin_types  = m_starting_pin_types;

                const auto endpoint_filter = [g, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g && !starting_pin_types.empty())
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
                                                  endpoint_filter,
                                                  endpoint_filter);

                if (neighborhood.is_error())
                {
                    return ERR_APPEND(neighborhood.get_error(), "cannot calculate feature " + to_string());
                }

                const std::vector<FEATURE_TYPE> feature = {FEATURE_TYPE(neighborhood.get().size())};

                return OK(feature);
            }

            std::string SequentialNeighborCount::to_string() const
            {
                std::string starting_pin_types_str = utils::join("_", m_starting_pin_types.begin(), m_starting_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "SequentialNeighborCount_" + std::to_string(m_depth) + "_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_"
                       + (starting_pin_types_str.empty() ? "None" : starting_pin_types_str) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            std::vector<std::string> SequentialNeighborCount::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                return {"sequential_neighbor_count"};
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
                const auto depth               = m_depth;
                const auto forbidden_pin_types = m_forbidden_pin_types;
                const auto starting_pin_types  = m_starting_pin_types;

                const auto endpoint_filter = [g, forbidden_pin_types, starting_pin_types, depth](const auto* ep, const auto& d) {
                    if (d > depth)
                    {
                        return false;
                    }

                    // do not filter out the pins with starting pin types at the start gate
                    // this for example allows us to investigate the neighborhood infront of the reset signal, while still forbidding reset pins for the propagation
                    if (ep->get_gate() == g && !starting_pin_types.empty())
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
                                                  endpoint_filter,
                                                  endpoint_filter);

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
                std::string starting_pin_types_str = utils::join("_", m_starting_pin_types.begin(), m_starting_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                std::string forbidden_pin_types_str = utils::join("_", m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), [](const PinType& pin_type) { return enum_to_string(pin_type); });

                return "NeighboringGateTypes_" + std::to_string(m_depth) + "_" + enum_to_string(m_direction) + "_" + std::to_string(m_directed) + "_"
                       + (starting_pin_types_str.empty() ? "None" : starting_pin_types_str) + "_" + (forbidden_pin_types_str.empty() ? "None" : forbidden_pin_types_str);
            }

            std::vector<std::string> NeighboringGateTypes::get_legend(Context& ctx) const
            {
                const auto& all_properties = ctx.get_possible_gate_type_properties();

                std::vector<std::string> legend;
                legend.reserve(all_properties.size());
                for (const auto& gtp : all_properties)
                {
                    legend.push_back("neighborhood_count_property_" + enum_to_string(gtp));
                }
                return legend;
            }
        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal