#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/progress_printer.h"
#include "machine_learning/features/gate_feature.h"
#include "machine_learning/utilities/normalization.h"
#include "graph_algorithm/algorithms/centrality.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {

            Result<std::vector<std::vector<FEATURE_TYPE>>> BetweennessCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
                const auto graph = ctx.get_original_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to get netlist graph");
                }

                const auto centrality = graph_algorithm::get_betweenness_centrality(graph.get(), gates, m_directed, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                auto centrality_values = centrality.get();

                if (m_normalize)
                {
                    const auto res = normalize_vector_min_max(centrality_values);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "cannot calculate feature " + to_string() + ": failed to normalize centrality values");
                    }
                }

                for (const auto& val : centrality_values)
                {
                    features.push_back({FEATURE_TYPE(val)});
                }

                return OK(features);
            }

            std::string BetweennessCentrality::to_string() const
            {
                return "BetweennessCentrality" + std::to_string(m_directed) + "_" + std::to_string(m_cutoff) + "_" + std::to_string(m_normalize);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> HarmonicCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
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
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                auto centrality_values = centrality.get();

                if (m_normalize)
                {
                    const auto res = normalize_vector_min_max(centrality_values);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "cannot calculate feature " + to_string() + ": failed to normalize centrality values");
                    }
                }

                for (const auto& val : centrality_values)
                {
                    features.push_back({FEATURE_TYPE(val)});
                }

                return OK(features);
            }

            std::string HarmonicCentrality::to_string() const
            {
                return "HarmonicCentrality" + enum_to_string(m_direction) + "_" + std::to_string(m_cutoff) + "_" + std::to_string(m_normalize);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> SequentialBetweennessCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
                const auto graph = ctx.get_sequential_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to get netlist graph");
                }

                const auto seq_gates  = graph.get()->get_included_gates();
                const auto centrality = graph_algorithm::get_betweenness_centrality(graph.get(), seq_gates, m_directed, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                auto centrality_values = centrality.get();

                if (m_normalize)
                {
                    const auto res = normalize_vector_min_max(centrality_values);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "cannot calculate feature " + to_string() + ": failed to normalize centrality values");
                    }
                }

                // assign each sequential gate part of the graph its centrality values
                std::unordered_map<Gate*, FEATURE_TYPE> seq_gate_to_centrality;
                for (u32 idx = 0; idx < seq_gates.size(); idx++)
                {
                    seq_gate_to_centrality.insert({seq_gates.at(idx), FEATURE_TYPE(centrality_values.at(idx))});
                }

                // build feature vector for all gates
                for (auto* gate : gates)
                {
                    if (const auto it = seq_gate_to_centrality.find(gate); it != seq_gate_to_centrality.end())
                    {
                        features.push_back({it->second});
                    }
                    else
                    {
                        features.push_back({FEATURE_TYPE(0)});
                    }
                }

                return OK(features);
            }

            std::string SequentialBetweennessCentrality::to_string() const
            {
                return "SequentialBetweennessCentrality" + std::to_string(m_directed) + "_" + std::to_string(m_cutoff) + "_" + std::to_string(m_normalize);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> SequentialHarmonicCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
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
                    return ERR_APPEND(graph.get_error(), "cannot calculate feature " + to_string() + ": failed to calculate centrality");
                }

                auto centrality_values = centrality.get();

                if (m_normalize)
                {
                    const auto res = normalize_vector_min_max(centrality_values);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "cannot calculate feature " + to_string() + ": failed to normalize centrality values");
                    }
                }

                // assign each sequential gate part of the graph its centrality values
                std::unordered_map<Gate*, FEATURE_TYPE> seq_gate_to_centrality;
                for (u32 idx = 0; idx < seq_gates.size(); idx++)
                {
                    seq_gate_to_centrality.insert({seq_gates.at(idx), FEATURE_TYPE(centrality_values.at(idx))});
                }

                // build feature vector for all gates
                for (auto* gate : gates)
                {
                    if (const auto it = seq_gate_to_centrality.find(gate); it != seq_gate_to_centrality.end())
                    {
                        features.push_back({it->second});
                    }
                    else
                    {
                        features.push_back({FEATURE_TYPE(0)});
                    }
                }

                return OK(features);
            }

            std::string SequentialHarmonicCentrality::to_string() const
            {
                return "SequentialHarmonicCentrality" + enum_to_string(m_direction) + "_" + std::to_string(m_cutoff) + "_" + std::to_string(m_normalize);
            }

        }    // namespace gate_feature
    }    // namespace machine_learning
}    // namespace hal