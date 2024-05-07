#include "graph_algorithm/algorithms/neighborhood.h"

namespace hal
{
    class Gate;

    namespace graph_algorithm
    {
        Result<std::vector<std::vector<u32>>> get_neighborhood(NetlistGraph* graph, std::vector<Gate*> start_gates, u32 order, NetlistGraph::Direction direction, u32 min_dist)
        {
            if (!graph)
            {
                return ERR("graph is a nullptr");
            }

            if (start_gates.empty())
            {
                return ERR("no start gates provided");
            }

            std::vector<u32> start_vertices;
            for (auto* g : start_gates)
            {
                if (const auto res = graph->get_vertex_from_gate(g); res.is_ok())
                {
                    start_vertices.push_back(res.get());
                }
                else
                {
                    return ERR(res.get_error());
                }
            }

            return get_neighborhood(graph, start_vertices, order, direction, min_dist);
        }

        Result<std::vector<std::vector<u32>>> get_neighborhood(NetlistGraph* graph, std::vector<u32> start_vertices, u32 order, NetlistGraph::Direction direction, u32 min_dist)
        {
            if (!graph)
            {
                return ERR("graph is a nullptr");
            }

            if (start_vertices.empty())
            {
                return ERR("no start vertices provided");
            }

            u32 num_vertices = igraph_vcount(graph->get_graph());
            if (std::any_of(start_vertices.begin(), start_vertices.end(), [num_vertices](const u32 v) { return v >= num_vertices; }))
            {
                return ERR("invalid vertex contained in start vertices");
            }

            igraph_vector_int_t v_vec;
            if (auto res = igraph_vector_int_init(&v_vec, start_vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            u32 v_count = 0;
            for (const auto v : start_vertices)
            {
                VECTOR(v_vec)[v_count++] = v;
            }

            igraph_vs_t v_sel;
            if (auto res = igraph_vs_vector(&v_sel, &v_vec); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&v_vec);
                return ERR(igraph_strerror(res));
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
                    igraph_vs_destroy(&v_sel);
                    igraph_vector_int_destroy(&v_vec);
                    return ERR("invalid direction 'NONE'");
            }

            igraph_vector_int_list_t neighborhoods_raw;
            if (auto res = igraph_vector_int_list_init(&neighborhoods_raw, 1); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_destroy(&v_vec);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_neighborhood(graph->get_graph(), &neighborhoods_raw, v_sel, order, mode, min_dist); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_destroy(&v_vec);
                igraph_vector_int_list_destroy(&neighborhoods_raw);
                return ERR(igraph_strerror(res));
            }

            std::vector<std::vector<u32>> neighborhoods;
            for (u32 i = 0; i < igraph_vector_int_list_size(&neighborhoods_raw); i++)
            {
                auto vec = igraph_vector_int_list_get_ptr(&neighborhoods_raw, i);

                u32 vec_size = igraph_vector_int_size(vec);
                std::vector<u32> tmp(vec_size);
                for (u32 j = 0; j < igraph_vector_int_size(vec); j++)
                {
                    tmp[j] = VECTOR(*vec)[j];
                }
                neighborhoods.push_back(std::move(tmp));
            }

            igraph_vs_destroy(&v_sel);
            igraph_vector_int_destroy(&v_vec);
            igraph_vector_int_list_destroy(&neighborhoods_raw);

            return OK(neighborhoods);
        }
    }    // namespace graph_algorithm
}    // namespace hal