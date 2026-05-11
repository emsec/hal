#include "graph_algorithm/algorithms/centrality.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/progress_printer.h"
#include "machine_learning/features/gate_feature.h"
#include "machine_learning/utilities/normalization.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            namespace
            {
                std::string normalizations_to_string(const std::vector<NormalizationType>& normalizations)
                {
                    std::string s;
                    for (const auto& n : normalizations)
                    {
                        s += "_" + enum_to_string(n);
                    }
                    return s;
                }

                /**
                 * Stack centrality values into per-gate feature rows by applying every requested
                 * normalization to a copy of the centrality values, in order. Centrality is
                 * computed once and only the (cheap) normalization step is repeated.
                 */
                Result<std::vector<std::vector<FEATURE_TYPE>>> stack_normalized_centrality(const std::vector<double>& centrality_values,
                                                                                           const std::vector<NormalizationType>& normalizations,
                                                                                           const u32 gate_count,
                                                                                           const std::string& feature_name)
                {
                    std::vector<std::vector<double>> normalized_columns;
                    normalized_columns.reserve(normalizations.size());

                    for (const auto& normalization : normalizations)
                    {
                        auto column   = centrality_values;
                        const auto rs = normalize_vector(normalization, column, gate_count);
                        if (rs.is_error())
                        {
                            return ERR_APPEND(rs.get_error(), "cannot calculate feature " + feature_name + ": failed to normalize centrality values");
                        }
                        normalized_columns.push_back(std::move(column));
                    }

                    std::vector<std::vector<FEATURE_TYPE>> features(centrality_values.size(), std::vector<FEATURE_TYPE>(normalizations.size()));
                    for (u32 col = 0; col < normalizations.size(); ++col)
                    {
                        for (u32 row = 0; row < centrality_values.size(); ++row)
                        {
                            features[row][col] = FEATURE_TYPE(normalized_columns[col][row]);
                        }
                    }
                    return OK(features);
                }
            }    // namespace

            Result<std::vector<std::vector<FEATURE_TYPE>>> BetweennessCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                const auto graph = ctx.get_original_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to get netlist graph");
                }

                const auto centrality = graph_algorithm::get_betweenness_centrality(graph.get(), gates, m_directed, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(centrality.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                return stack_normalized_centrality(centrality.get(), m_normalizations, ctx.get_gates().size(), to_string());
            }

            std::string BetweennessCentrality::to_string() const
            {
                return "BetweennessCentrality" + std::to_string(m_directed) + "_" + std::to_string(m_cutoff) + normalizations_to_string(m_normalizations);
            }

            std::vector<std::string> BetweennessCentrality::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                std::vector<std::string> legend;
                legend.reserve(m_normalizations.size());
                for (const auto& n : m_normalizations)
                {
                    legend.push_back("betweenness_centrality_" + enum_to_string(n));
                }
                return legend;
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> HarmonicCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                const auto graph = ctx.get_original_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to get netlist graph");
                }

                graph_algorithm::NetlistGraph::Direction direction;
                switch (m_direction)
                {
                    case PinDirection::input:
                        direction = graph_algorithm::NetlistGraph::Direction::IN;
                        break;
                    case PinDirection::output:
                        direction = graph_algorithm::NetlistGraph::Direction::OUT;
                        break;
                    case PinDirection::inout:
                        direction = graph_algorithm::NetlistGraph::Direction::ALL;
                        break;
                    default:
                        return ERR("invalid pin direction " + enum_to_string(m_direction));
                }

                const auto centrality = graph_algorithm::get_harmonic_centrality(graph.get(), gates, direction, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(centrality.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                return stack_normalized_centrality(centrality.get(), m_normalizations, ctx.get_gates().size(), to_string());
            }

            std::string HarmonicCentrality::to_string() const
            {
                return "HarmonicCentrality" + enum_to_string(m_direction) + "_" + std::to_string(m_cutoff) + normalizations_to_string(m_normalizations);
            }

            std::vector<std::string> HarmonicCentrality::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                std::vector<std::string> legend;
                legend.reserve(m_normalizations.size());
                for (const auto& n : m_normalizations)
                {
                    legend.push_back("harmonic_centrality_" + enum_to_string(n));
                }
                return legend;
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> SequentialBetweennessCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                const auto graph = ctx.get_sequential_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to get netlist graph");
                }

                const auto seq_gates  = graph.get()->get_included_gates();
                const auto centrality = graph_algorithm::get_betweenness_centrality(graph.get(), seq_gates, m_directed, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(centrality.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                const auto seq_features_res = stack_normalized_centrality(centrality.get(), m_normalizations, ctx.get_gates().size(), to_string());
                if (seq_features_res.is_error())
                {
                    return seq_features_res;
                }
                const auto seq_features = seq_features_res.get();

                std::unordered_map<Gate*, std::vector<FEATURE_TYPE>> seq_gate_to_centrality;
                for (u32 idx = 0; idx < seq_gates.size(); idx++)
                {
                    seq_gate_to_centrality.insert({seq_gates.at(idx), seq_features.at(idx)});
                }

                std::vector<std::vector<FEATURE_TYPE>> features;
                features.reserve(gates.size());
                const std::vector<FEATURE_TYPE> zero_row(m_normalizations.size(), FEATURE_TYPE(0));
                for (auto* gate : gates)
                {
                    if (const auto it = seq_gate_to_centrality.find(gate); it != seq_gate_to_centrality.end())
                    {
                        features.push_back(it->second);
                    }
                    else
                    {
                        features.push_back(zero_row);
                    }
                }

                return OK(features);
            }

            std::string SequentialBetweennessCentrality::to_string() const
            {
                return "SequentialBetweennessCentrality" + std::to_string(m_directed) + "_" + std::to_string(m_cutoff) + normalizations_to_string(m_normalizations);
            }

            std::vector<std::string> SequentialBetweennessCentrality::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                std::vector<std::string> legend;
                legend.reserve(m_normalizations.size());
                for (const auto& n : m_normalizations)
                {
                    legend.push_back("sequential_betweenness_centrality_" + enum_to_string(n));
                }
                return legend;
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> SequentialHarmonicCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                const auto graph = ctx.get_sequential_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to get netlist graph");
                }

                graph_algorithm::NetlistGraph::Direction direction;
                switch (m_direction)
                {
                    case PinDirection::input:
                        direction = graph_algorithm::NetlistGraph::Direction::IN;
                        break;
                    case PinDirection::output:
                        direction = graph_algorithm::NetlistGraph::Direction::OUT;
                        break;
                    case PinDirection::inout:
                        direction = graph_algorithm::NetlistGraph::Direction::ALL;
                        break;
                    default:
                        return ERR("invalid pin direction " + enum_to_string(m_direction));
                }

                const auto seq_gates  = graph.get()->get_included_gates();
                const auto centrality = graph_algorithm::get_harmonic_centrality(graph.get(), seq_gates, direction, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(centrality.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                const auto seq_features_res = stack_normalized_centrality(centrality.get(), m_normalizations, ctx.get_gates().size(), to_string());
                if (seq_features_res.is_error())
                {
                    return seq_features_res;
                }
                const auto seq_features = seq_features_res.get();

                std::unordered_map<Gate*, std::vector<FEATURE_TYPE>> seq_gate_to_centrality;
                for (u32 idx = 0; idx < seq_gates.size(); idx++)
                {
                    seq_gate_to_centrality.insert({seq_gates.at(idx), seq_features.at(idx)});
                }

                std::vector<std::vector<FEATURE_TYPE>> features;
                features.reserve(gates.size());
                const std::vector<FEATURE_TYPE> zero_row(m_normalizations.size(), FEATURE_TYPE(0));
                for (auto* gate : gates)
                {
                    if (const auto it = seq_gate_to_centrality.find(gate); it != seq_gate_to_centrality.end())
                    {
                        features.push_back(it->second);
                    }
                    else
                    {
                        features.push_back(zero_row);
                    }
                }

                return OK(features);
            }

            std::string SequentialHarmonicCentrality::to_string() const
            {
                return "SequentialHarmonicCentrality" + enum_to_string(m_direction) + "_" + std::to_string(m_cutoff) + normalizations_to_string(m_normalizations);
            }

            std::vector<std::string> SequentialHarmonicCentrality::get_legend(Context& ctx) const
            {
                UNUSED(ctx);
                std::vector<std::string> legend;
                legend.reserve(m_normalizations.size());
                for (const auto& n : m_normalizations)
                {
                    legend.push_back("sequential_harmonic_centrality_" + enum_to_string(n));
                }
                return legend;
            }

        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal
