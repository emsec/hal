#include "graph_algorithm/algorithms/neighborhood.h"

namespace hal
{
    class Gate;

    namespace graph_algorithm
    {
        Result<std::vector<std::vector<u32>>> get_neighborhood(NetlistGraph* graph, const std::vector<Gate*>& start_gates, u32 order, NetlistGraph::Direction direction, u32 min_dist)
        {
            if (!graph)
            {
                return ERR("graph is a nullptr");
            }

            if (start_gates.empty())
            {
                return ERR("no start gates provided");
            }

            igraph_vector_int_t i_gates;
            if (auto res = graph->get_vertices_from_gates_igraph(start_gates); res.is_ok())
            {
                i_gates = std::move(res.get());
            }
            else
            {
                return ERR(res.get_error());
            }

            auto res = get_neighborhood_igraph(graph, &i_gates, order, direction, min_dist);

            igraph_vector_int_destroy(&i_gates);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<std::vector<u32>>> get_neighborhood(NetlistGraph* graph, const std::vector<u32>& start_vertices, u32 order, NetlistGraph::Direction direction, u32 min_dist)
        {
            if (!graph)
            {
                return ERR("graph is a nullptr");
            }

            if (start_vertices.empty())
            {
                return ERR("no start vertices provided");
            }

            igraph_vector_int_t i_gates;
            if (auto res = igraph_vector_int_init(&i_gates, start_vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < start_vertices.size(); i++)
            {
                VECTOR(i_gates)[i] = start_vertices.at(i);
            }

            auto res = get_neighborhood_igraph(graph, &i_gates, order, direction, min_dist);

            igraph_vector_int_destroy(&i_gates);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<std::vector<u32>>> get_neighborhood_igraph(NetlistGraph* graph, const igraph_vector_int_t* start_gates, u32 order, NetlistGraph::Direction direction, u32 min_dist)
        {
            if (!graph)
            {
                return ERR("graph is a nullptr");
            }

            igraph_vs_t v_sel = igraph_vss_vector(start_gates);
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
                    return ERR("invalid direction 'NONE'");
            }

            igraph_vector_int_list_t neighborhoods_raw;
            if (auto res = igraph_vector_int_list_init(&neighborhoods_raw, 1); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_neighborhood(graph->get_graph(), &neighborhoods_raw, v_sel, order, mode, min_dist); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_list_destroy(&neighborhoods_raw);
                return ERR(igraph_strerror(res));
            }

            std::vector<std::vector<u32>> neighborhoods;
            const u32 num_neighborhoods = igraph_vector_int_list_size(&neighborhoods_raw);
            for (u32 i = 0; i < num_neighborhoods; i++)
            {
                auto vec = igraph_vector_int_list_get_ptr(&neighborhoods_raw, i);

                const u32 vec_size = igraph_vector_int_size(vec);
                std::vector<u32> tmp(vec_size);
                for (u32 j = 0; j < vec_size; j++)
                {
                    tmp[j] = VECTOR(*vec)[j];
                }
                neighborhoods.push_back(std::move(tmp));
            }

            igraph_vs_destroy(&v_sel);
            igraph_vector_int_list_destroy(&neighborhoods_raw);

            return OK(neighborhoods);
        }
    }    // namespace graph_algorithm
}    // namespace hal