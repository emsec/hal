#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "machine_learning/features/gate_feature.h"

#include <vector>

#define MAX_DISTANCE 255

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            const Result<NetlistAbstraction*> FeatureContext::get_sequential_abstraction()
            {
                if (!m_sequential_abstraction.has_value())
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

                    m_sequential_abstraction = sequential_abstraction_res.get();

                    // TODO remove debug print
                    // std::cout << "Built abstraction" << std::endl;
                }

                return OK(&m_sequential_abstraction.value());
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

            const std::vector<GateTypeProperty>& FeatureContext::get_possible_gate_type_properties()
            {
                if (!m_possible_gate_type_properties.has_value())
                {
                    std::set<GateTypeProperty> properties;

                    for (const auto& [_name, gt] : nl->get_gate_library()->get_gate_types())
                    {
                        const auto gt_properties = gt->get_properties();
                        properties.insert(gt_properties.begin(), gt_properties.end());
                    }

                    // for (auto& [gtp, _name] : EnumStrings<GateTypeProperty>::data)
                    // {
                    //     UNUSED(_name);
                    //     properties.insert(gtp);
                    // }

                    auto properties_vec = utils::to_vector(properties);
                    // sort alphabetically
                    std::sort(properties_vec.begin(), properties_vec.end(), [](const auto& a, const auto& b) { return enum_to_string(a) < enum_to_string(b); });
                    m_possible_gate_type_properties = properties_vec;
                }

                return m_possible_gate_type_properties.value();
            }

            Result<std::vector<u32>> ConnectedGlobalIOs::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                UNUSED(fc);

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

                return OK({connected_global_inputs, connected_global_outputs});
            }

            std::string ConnectedGlobalIOs::to_string() const
            {
                return "ConnectedGlobalIOs";
            }

            Result<std::vector<u32>> DistanceGlobalIO::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                // necessary workaround to please compiler
                const auto& direction                                = m_direction;
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_original_abstraction();
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

                return OK({std::min(distance.get().value(), u32(MAX_DISTANCE))});
            }

            std::string DistanceGlobalIO::to_string() const
            {
                return "DistanceGlobalIO";
            }

            Result<std::vector<u32>> SequentialDistanceGlobalIO::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                // necessary workaround to please compiler
                const auto& direction                                = m_direction;
                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_sequential_abstraction();
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

                return OK({std::min(distance.get().value(), u32(MAX_DISTANCE))});
            }

            std::string SequentialDistanceGlobalIO::to_string() const
            {
                return "SequentialDistanceGlobalIO";
            }

            Result<std::vector<u32>> IODegrees::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                u32 input_io_degree  = g->get_fan_in_nets().size();
                u32 output_io_degree = g->get_fan_out_nets().size();

                return OK({input_io_degree, output_io_degree});
            }

            std::string IODegrees::to_string() const
            {
                return "IODegrees";
            }

            Result<std::vector<u32>> GateTypeOneHot::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                const auto& all_properties = fc.get_possible_gate_type_properties();

                // TODO remove debug print
                // std::cout << "Got following gate type properties: " << std::endl;
                // for (const auto& gtp : all_properties)
                // {
                //     std::cout << enum_to_string(gtp) << std::endl;
                // }

                std::vector<u32> feature = std::vector<u32>(all_properties.size(), 0);

                for (const auto& gtp : g->get_type()->get_properties())
                {
                    const u32 index = std::distance(all_properties.begin(), std::find(all_properties.begin(), all_properties.end(), gtp));
                    feature.at(index) += 1;
                }

                return OK(feature);
            }

            std::string GateTypeOneHot::to_string() const
            {
                return "GateTypeOneHot";
            }

            Result<std::vector<u32>> NeighboringGateTypes::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                const auto& all_properties = fc.get_possible_gate_type_properties();

                const hal::Result<hal::NetlistAbstraction*> nl_abstr = fc.get_original_abstraction();
                if (nl_abstr.is_error())
                {
                    return ERR_APPEND(nl_abstr.get_error(), "cannot calculate feature " + to_string() + ": failed to get original netlist abstraction");
                }

                std::vector<u32> feature = std::vector<u32>(all_properties.size(), 0);

                // fix to make compiler happpy
                const auto depth = m_depth;

                const auto neighborhood = NetlistAbstractionDecorator(*(nl_abstr.get()))
                                              .get_next_matching_gates_until(
                                                  g,
                                                  [](const auto* g) { return true; },
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
                        feature.at(index) += 1;
                    }
                }

                return OK(feature);
            }

            std::string NeighboringGateTypes::to_string() const
            {
                return "NeighboringGateTypes";
            }

            Result<std::vector<u32>> build_feature_vec(const std::vector<const GateFeature*>& features, const Gate* g)
            {
                FeatureContext fc(g->get_netlist());
                return build_feature_vec(features, g);
            }

            Result<std::vector<u32>> build_feature_vec(FeatureContext& fc, const std::vector<const GateFeature*>& features, const Gate* g)
            {
                std::vector<u32> feature_vec;

                for (const auto& gf : features)
                {
                    // TODO remove debug print
                    // std::cout << "Calculating feature: " << gf->to_string() << std::endl;

                    const auto new_features = gf->calculate_feature(fc, g);
                    if (new_features.is_error())
                    {
                        return ERR_APPEND(new_features.get_error(),
                                          "cannot build feature vector for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + ": failde to calculate feature " + gf->to_string());
                    }

                    feature_vec.insert(feature_vec.end(), new_features.get().begin(), new_features.get().end());
                }

                return OK(feature_vec);
            }

            Result<std::vector<std::vector<u32>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates)
            {
                if (gates.empty())
                {
                    return OK({});
                }

                FeatureContext fc(gates.front()->get_netlist());
                return build_feature_vecs(features, gates);
            }

            Result<std::vector<std::vector<u32>>> build_feature_vecs(FeatureContext& fc, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates)
            {
                std::vector<std::vector<u32>> feature_vecs;

                for (const auto& g : gates)
                {
                    const auto feature_vec = build_feature_vec(fc, features, g);
                    if (feature_vec.is_error())
                    {
                        return ERR_APPEND(feature_vec.get_error(), "cannot build feature vecs: failed to build feature vector for gate " + g->get_name() + " with ID " + std::to_string(g->get_id()));
                    }

                    feature_vecs.push_back(feature_vec.get());
                }

                return OK(feature_vecs);
            }
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal