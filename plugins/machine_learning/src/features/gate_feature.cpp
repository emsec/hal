#include "machine_learning/features/gate_feature.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

#include <vector>

#define MAX_DISTANCE 255

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
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

                    // TODO remove debug print
                    std::cout << "Built abstraction" << std::endl;
                }

                return m_seqential_abstraction.value();
            }

            const std::vector<GateTypeProperty>& FeatureContext::get_possible_gate_type_properties()
            {
                if (!m_possible_gate_type_properties.has_value())
                {
                    std::set<GateTypeProperty> properties;

                    // for (const auto& [_name, gt] : nl->get_gate_library()->get_gate_types())
                    // {
                    //     properties.insert(gt->get_properties().begin(), gt->get_properties().end());
                    // }

                    for (auto& [gtp, _name] : EnumStrings<GateTypeProperty>::data)
                    {
                        UNUSED(_name);
                        properties.insert(gtp);
                    }

                    auto properties_vec = utils::to_vector(properties);
                    // sort alphabetically
                    std::sort(properties_vec.begin(), properties_vec.end(), [](const auto& a, const auto& b) { return enum_to_string(a) < enum_to_string(b); });
                    m_possible_gate_type_properties = properties_vec;
                }

                return m_possible_gate_type_properties.value();
            }

            std::vector<u32> ConnectedGlobalIOs::calculate_feature(FeatureContext& fc, const Gate* g) const
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

                return {connected_global_inputs, connected_global_outputs};
            }

            std::string ConnectedGlobalIOs::get_name() const
            {
                return "ConnectedGlobalIOs";
            }

            std::vector<u32> DistanceGlobalIO::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                // necessary workaround to please compiler
                const auto& direction = m_direction;
                const auto distance   = NetlistTraversalDecorator(*fc.nl).get_shortest_path_distance(
                    g, [direction](const auto* ep) { return (direction == PinDirection::output) ? ep->get_net()->is_global_output_net() : ep->get_net()->is_global_input_net(); }, m_direction);

                if (distance.is_error())
                {
                    log_error("machine_learning", "{}", distance.get_error().get());
                }

                if (!distance.get().has_value())
                {
                    return {MAX_DISTANCE};
                }

                return {std::min(distance.get().value(), u32(MAX_DISTANCE))};
            }

            std::string DistanceGlobalIO::get_name() const
            {
                return "DistanceGlobalIO";
            }

            std::vector<u32> SequentialDistanceGlobalIO::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                // necessary workaround to please compiler
                const auto& direction = m_direction;
                const auto distance   = NetlistAbstractionDecorator(fc.get_sequential_abstraction())
                                          .get_shortest_path_distance(
                                              g,
                                              [direction](const auto* ep, const auto& nla) {
                                                  const auto global_io_connections = (direction == PinDirection::output) ? nla.get_global_output_successors(ep) : nla.get_global_input_predecessors(ep);
                                                  return !global_io_connections.empty();
                                              },
                                              m_direction);

                if (distance.is_error())
                {
                    log_error("machine_learning", "{}", distance.get_error().get());
                }

                if (!distance.get().has_value())
                {
                    return {MAX_DISTANCE};
                }

                return {std::min(distance.get().value(), u32(MAX_DISTANCE))};
            }

            std::string SequentialDistanceGlobalIO::get_name() const
            {
                return "SequentialDistanceGlobalIO";
            }

            std::vector<u32> IODegrees::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                u32 input_io_degree  = g->get_fan_in_nets().size();
                u32 output_io_degree = g->get_fan_out_nets().size();

                return {input_io_degree, output_io_degree};
            }

            std::string IODegrees::get_name() const
            {
                return "IODegrees";
            }

            std::vector<u32> GateTypeOneHot::calculate_feature(FeatureContext& fc, const Gate* g) const
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

                return feature;
            }

            std::string GateTypeOneHot::get_name() const
            {
                return "GateTypeOneHot";
            }

            std::vector<u32> NeighboringGateTypes::calculate_feature(FeatureContext& fc, const Gate* g) const
            {
                const auto& all_properties = fc.get_possible_gate_type_properties();

                std::vector<u32> feature = std::vector<u32>(all_properties.size(), 0);

                std::set<Gate*> neighborhood;

                // fix to make compiler happpy
                const auto depth = m_depth;

                if (m_direction == PinDirection::input || m_direction == PinDirection::inout)
                {
                    const auto in_gates = NetlistTraversalDecorator(*fc.nl).get_next_matching_gates_until(
                        g,
                        false,
                        [](const auto* g) { return true; },
                        true,
                        [depth](const auto* _ep, const auto current_depth) {
                            UNUSED(_ep);
                            return current_depth <= depth;
                        },
                        nullptr);

                    if (in_gates.is_error())
                    {
                        log_error("machine_learining", "cannot build NeighboringGateTypes feature: {}", in_gates.get_error().get());
                    }

                    neighborhood.insert(in_gates.get().begin(), in_gates.get().end());
                }

                if (m_direction == PinDirection::output || m_direction == PinDirection::inout)
                {
                    const auto in_gates = NetlistTraversalDecorator(*fc.nl).get_next_matching_gates_until(
                        g,
                        true,
                        [](const auto* g) { return true; },
                        true,
                        [depth](const auto* _ep, const auto current_depth) {
                            UNUSED(_ep);
                            return current_depth <= depth;
                        },
                        nullptr);

                    if (in_gates.is_error())
                    {
                        log_error("machine_learining", "cannot build NeighboringGateTypes feature: {}", in_gates.get_error().get());
                    }

                    neighborhood.insert(in_gates.get().begin(), in_gates.get().end());
                }

                for (const auto& gn : neighborhood)
                {
                    for (const auto& gtp : gn->get_type()->get_properties())
                    {
                        const u32 index = std::distance(all_properties.begin(), std::find(all_properties.begin(), all_properties.end(), gtp));
                        feature.at(index) += 1;
                    }
                }

                return feature;
            }

            std::string NeighboringGateTypes::get_name() const
            {
                return "NeighboringGateTypes";
            }

            std::vector<u32> build_feature_vec(const std::vector<const GateFeature*>& features, const Gate* g)
            {
                FeatureContext fc(g->get_netlist());
                return build_feature_vec(features, g);
            }

            std::vector<u32> build_feature_vec(FeatureContext& fc, const std::vector<const GateFeature*>& features, const Gate* g)
            {
                std::vector<u32> feature_vec;

                for (const auto& gf : features)
                {
                    const auto new_features = gf->calculate_feature(fc, g);
                    feature_vec.insert(feature_vec.end(), new_features.begin(), new_features.end());
                }

                return feature_vec;
            }

            std::vector<std::vector<u32>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates)
            {
                if (gates.empty())
                {
                    return {};
                }

                FeatureContext fc(gates.front()->get_netlist());
                return build_feature_vecs(features, gates);
            }

            std::vector<std::vector<u32>> build_feature_vecs(FeatureContext& fc, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates)
            {
                std::vector<std::vector<u32>> feature_vecs;

                for (const auto& g : gates)
                {
                    feature_vecs.push_back(build_feature_vec(fc, features, g));
                }

                return feature_vecs;
            }
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal