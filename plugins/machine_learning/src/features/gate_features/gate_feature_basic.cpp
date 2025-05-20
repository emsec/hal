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
        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal