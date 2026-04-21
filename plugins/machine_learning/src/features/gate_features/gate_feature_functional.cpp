#include "hal_core/defines.h"
#include "hal_core/netlist/net.h"
#include "machine_learning/features/gate_feature.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            Result<std::vector<FEATURE_TYPE>> BooleanInfluence::calculate_feature(Context& ctx, const Gate* g) const
            {
                // Only meaningful for sequential gates.
                if (!g->get_type()->has_property(GateTypeProperty::sequential))
                {
                    return OK(std::vector<FEATURE_TYPE>(m_moments.size(), FEATURE_TYPE(0)));
                }

                // 1. Get the output nets of the gate.
                std::vector<Net*> output_nets;
                for (const auto* ep : g->get_fan_out_endpoints())
                {
                    if (ep->get_pin() == nullptr)
                    {
                        continue;
                    }
                    // if (std::find(m_forbidden_pin_types.begin(), m_forbidden_pin_types.end(), ep->get_pin()->get_type()) != m_forbidden_pin_types.end())
                    // {
                    //     continue;
                    // }
                    Net* net = ep->get_net();
                    if (net != nullptr)
                    {
                        output_nets.push_back(net);
                    }
                }

                if (output_nets.empty())
                {
                    return OK(std::vector<FEATURE_TYPE>(m_moments.size(), FEATURE_TYPE(0)));
                }

                // 2. Find all the nets whose Boolean function is influenced by the gate's output nets, and get their Boolean functions.
                const auto sequential_influenced_nets_res = ctx.get_sequential_influenced_nets();
                if (sequential_influenced_nets_res.is_error())
                {
                    return ERR_APPEND(sequential_influenced_nets_res.get_error(),
                                      "unable to calculate Boolean influence for gate " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                          + ": failed to get sequential influenced nets from context.");
                }
                const auto* sequential_influenced_nets = sequential_influenced_nets_res.get();

                std::vector<Net*> influenced_nets;
                for (const auto& output_net : output_nets)
                {
                    const auto influenced_nets_res = sequential_influenced_nets->find(output_net);
                    if (influenced_nets_res == sequential_influenced_nets->end())
                    {
                        // This output net does not appear in any cached sequential subgraph function —
                        // either it genuinely drives no sequential inputs, or all downstream subgraphs
                        // exceeded m_max_boolean_function_input_size and were excluded from the cache.
                        continue;
                    }

                    influenced_nets.insert(influenced_nets.end(), influenced_nets_res->second.begin(), influenced_nets_res->second.end());
                }

                if (influenced_nets.empty())
                {
                    return OK(std::vector<FEATURE_TYPE>(m_moments.size(), FEATURE_TYPE(0)));
                }

                // 3. Collect all influences for the influenced nets, and calculate the requested statistical moments.
                const auto sequential_boolean_influence_res = ctx.get_sequential_boolean_influences();
                if (sequential_boolean_influence_res.is_error())
                {
                    return ERR_APPEND(sequential_boolean_influence_res.get_error(),
                                      "unable to calculate Boolean influence for gate " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                          + ": failed to get sequential Boolean influence from context.");
                }
                const auto* sequential_boolean_influences = sequential_boolean_influence_res.get();

                std::vector<double> influences;
                for (const auto& influenced_net : influenced_nets)
                {
                    const auto it = sequential_boolean_influences->find(influenced_net);
                    if (it == sequential_boolean_influences->end())
                    {
                        return ERR("unable to calculate Boolean influence for gate " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                   + ": failed to find Boolean influence for influenced net " + influenced_net->get_name() + " with ID " + std::to_string(influenced_net->get_id()) + " in cache.");
                    }

                    for (auto& output_net : output_nets)
                    {
                        const auto net_influence = it->second.find(output_net);
                        if (net_influence == it->second.end())
                        {
                            return ERR("unable to calculate Boolean influence for gate " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                       + ": failed to find Boolean influence for influenced net " + influenced_net->get_name() + " with ID " + std::to_string(influenced_net->get_id())
                                       + " and output net " + output_net->get_name() + " with ID " + std::to_string(output_net->get_id()) + " in cache.");
                        }
                        influences.push_back(net_influence->second);
                    }
                }

                // 4. Calculate the requested statistical moments from the collected influences.
                std::vector<FEATURE_TYPE> result;
                for (const auto& moment : m_moments)
                {
                    const auto moment_result = calculate_statistical_moment(moment, std::vector<FEATURE_TYPE>(influences.begin(), influences.end()));
                    if (moment_result.is_error())
                    {
                        return ERR_APPEND(moment_result.get_error(),
                                          "unable to calculate Boolean influence for gate " + g->get_name() + " with ID " + std::to_string(g->get_id())
                                              + ": failed to calculate statistical moment " + enum_to_string(moment) + " from influences.");
                    }
                    result.push_back(moment_result.get());
                }

                return OK(result);
            }

            std::string BooleanInfluence::to_string() const
            {
                std::string moments_str = utils::join("_", m_moments.begin(), m_moments.end(), [](const StatisticalMoment& moment) { return enum_to_string(moment); });
                return "BooleanInfluence_" + (moments_str.empty() ? "None" : moments_str);
            }

        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal
