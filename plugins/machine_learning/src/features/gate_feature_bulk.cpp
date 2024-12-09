#include "graph_algorithm/algorithms/centrality.h"
#include "hal_core/netlist/gate.h"
#include "machine_learning/features/gate_feature_bulk.h"

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
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to get netlist graph");
                }

                const auto centrality = graph_algorithm::get_betweenness_centrality(graph.get(), gates, m_directed, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to calculate centrality");
                }

                for (const auto& val : centrality.get())
                {
                    features.push_back({FEATURE_TYPE(val)});
                }

                return OK(features);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> HarmonicCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
                const auto graph = ctx.get_original_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to get netlist graph");
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
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to calculate centrality");
                }

                for (const auto& val : centrality.get())
                {
                    features.push_back({FEATURE_TYPE(val)});
                }

                return OK(features);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> SequentialBetweennessCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
                const auto graph = ctx.get_sequential_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to get netlist graph");
                }

                const auto centrality = graph_algorithm::get_betweenness_centrality(graph.get(), gates, m_directed, m_cutoff);
                if (centrality.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to calculate centrality");
                }

                for (const auto& val : centrality.get())
                {
                    features.push_back({FEATURE_TYPE(val)});
                }

                return OK(features);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> SequentialHarmonicCentrality::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                std::vector<std::vector<FEATURE_TYPE>> features;
                const auto graph = ctx.get_sequential_netlist_graph();
                if (graph.is_error())
                {
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to get netlist graph");
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
                    return ERR_APPEND(graph.get_error(), "cannot calcualte feature " + to_string() + ": failed to calculate centrality");
                }

                for (const auto& val : centrality.get())
                {
                    features.push_back({FEATURE_TYPE(val)});
                }

                return OK(features);
            }
        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal