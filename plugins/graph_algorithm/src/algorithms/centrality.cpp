#include "graph_algorithm/algorithms/centrality.h"

namespace hal
{
    namespace graph_algorithm
    {
        Result<std::vector<double>> get_harmonic_centrality(const NetlistGraph* graph, const std::vector<Gate*>& gates, const NetlistGraph::Direction direction, const i32 cutoff)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (gates.empty())
            {
                return ERR("no gates provided");
            }

            igraph_vector_int_t i_vertices;
            if (auto res = graph->get_vertices_from_gates_igraph(gates); res.is_ok())
            {
                i_vertices = std::move(res.get());
            }
            else
            {
                return ERR(res.get_error());
            }

            auto res = get_harmonic_centrality(graph, &i_vertices, direction);

            igraph_vector_int_destroy(&i_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<double>> get_harmonic_centrality(const NetlistGraph* graph, const std::vector<u32>& vertices, const NetlistGraph::Direction direction, const i32 cutoff)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (vertices.empty())
            {
                return ERR("no destination vertices provided");
            }

            igraph_vector_int_t i_vertices;
            if (auto res = igraph_vector_int_init(&i_vertices, vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < vertices.size(); i++)
            {
                VECTOR(i_vertices)[i] = vertices.at(i);
            }

            auto res = get_harmonic_centrality(graph, &i_vertices, direction);

            igraph_vector_int_destroy(&i_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<double>> get_harmonic_centrality(const NetlistGraph* graph, const igraph_vector_int_t* vertices, const NetlistGraph::Direction direction, const i32 cutoff)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            igraph_neimode_t mode;
            switch (direction)
            {
                case NetlistGraph::Direction::IN:
                    mode = IGRAPH_IN;
                    break;
                case NetlistGraph::Direction::OUT:
                    mode = IGRAPH_OUT;
                    break;
                case NetlistGraph::Direction::ALL:
                    mode = IGRAPH_ALL;
                    break;
                case NetlistGraph::Direction::NONE:
                    return ERR("invalid direction 'NONE'");
                default:
                    return ERR("unhandled direction");
            }

            igraph_bool_t i_normalized = true;
            igraph_real_t i_cutoff     = double(cutoff);

            igraph_vs_t v_sel = igraph_vss_vector(vertices);
            igraph_vector_t scores;
            if (auto res = igraph_vector_init(&scores, 0); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_harmonic_centrality_cutoff(graph->get_graph(), &scores, v_sel, mode, nullptr, i_normalized, i_cutoff); res != IGRAPH_SUCCESS)
            {
                igraph_vector_destroy(&scores);
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            // Convert igraph_vector_t to std::vector<double>
            std::vector<double> result;
            result.reserve(igraph_vector_size(&scores));
            for (int i = 0; i < igraph_vector_size(&scores); ++i)
            {
                result.push_back(VECTOR(scores)[i]);
            }

            // Cleanup
            igraph_vector_destroy(&scores);
            igraph_vs_destroy(&v_sel);

            return OK(result);
        }

        Result<std::vector<double>> get_betweenness_centrality(const NetlistGraph* graph, const std::vector<Gate*>& gates, const bool directed, const i32 cutoff)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (gates.empty())
            {
                return ERR("no gates provided");
            }

            igraph_vector_int_t i_vertices;
            if (auto res = graph->get_vertices_from_gates_igraph(gates); res.is_ok())
            {
                i_vertices = std::move(res.get());
            }
            else
            {
                return ERR(res.get_error());
            }

            auto res = get_betweenness_centrality(graph, &i_vertices, directed);

            igraph_vector_int_destroy(&i_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<double>> get_betweenness_centrality(const NetlistGraph* graph, const std::vector<u32>& vertices, const bool directed, const i32 cutoff)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (vertices.empty())
            {
                return ERR("no destination vertices provided");
            }

            igraph_vector_int_t i_vertices;
            if (auto res = igraph_vector_int_init(&i_vertices, vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < vertices.size(); i++)
            {
                VECTOR(i_vertices)[i] = vertices.at(i);
            }

            auto res = get_betweenness_centrality(graph, &i_vertices, directed);

            igraph_vector_int_destroy(&i_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<double>> get_betweenness_centrality(const NetlistGraph* graph, const igraph_vector_int_t* vertices, const bool directed, const i32 cutoff)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            igraph_bool_t i_directed = directed;
            igraph_real_t i_cutoff   = double(cutoff);

            igraph_vs_t v_sel = igraph_vss_vector(vertices);
            igraph_vector_t scores;
            if (auto res = igraph_vector_init(&scores, 0); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_betweenness_cutoff(graph->get_graph(), &scores, v_sel, i_directed, nullptr, i_cutoff); res != IGRAPH_SUCCESS)
            {
                igraph_vector_destroy(&scores);
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            // Convert igraph_vector_t to std::vector<double>
            std::vector<double> result;
            result.reserve(igraph_vector_size(&scores));
            for (int i = 0; i < igraph_vector_size(&scores); ++i)
            {
                result.push_back(VECTOR(scores)[i]);
            }

            // Cleanup
            igraph_vector_destroy(&scores);
            igraph_vs_destroy(&v_sel);

            return OK(result);
        }
    }    // namespace graph_algorithm
}    // namespace hal