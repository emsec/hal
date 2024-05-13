#include "graph_algorithm/algorithms/shortest_path.h"

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/netlist/gate.h"

namespace hal
{
    namespace graph_algorithm
    {
        Result<std::vector<std::vector<u32>>> get_shortest_paths(NetlistGraph* graph, Gate* from_gate, const std::vector<Gate*>& to_gates, NetlistGraph::Direction direction)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (!from_gate)
            {
                return ERR("no source gate provided");
            }

            if (to_gates.empty())
            {
                return ERR("no destination gates provided");
            }

            u32 from_vertex;
            if (auto res = graph->get_vertex_from_gate(from_gate); res.is_ok())
            {
                from_vertex = res.get();
            }
            else
            {
                return ERR(res.get_error());
            }

            igraph_vector_int_t i_to_vertices;
            if (auto res = graph->get_vertices_from_gates_igraph(to_gates); res.is_ok())
            {
                i_to_vertices = std::move(res.get());
            }
            else
            {
                return ERR(res.get_error());
            }

            auto res = get_shortest_paths_igraph(graph, from_vertex, &i_to_vertices, direction);

            igraph_vector_int_destroy(&i_to_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<std::vector<u32>>> get_shortest_paths(NetlistGraph* graph, u32 from_vertex, const std::vector<u32>& to_vertices, NetlistGraph::Direction direction)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (to_vertices.empty())
            {
                return ERR("no destination vertices provided");
            }

            igraph_vector_int_t i_to_vertices;
            if (auto res = igraph_vector_int_init(&i_to_vertices, to_vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < to_vertices.size(); i++)
            {
                VECTOR(i_to_vertices)[i] = to_vertices.at(i);
            }

            auto res = get_shortest_paths_igraph(graph, from_vertex, &i_to_vertices, direction);

            igraph_vector_int_destroy(&i_to_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<std::vector<u32>>> get_shortest_paths_igraph(NetlistGraph* graph, u32 from_vertex, const igraph_vector_int_t* to_vertices, NetlistGraph::Direction direction)
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
            }

            igraph_vs_t v_sel = igraph_vss_vector(to_vertices);
            igraph_vector_int_list_t paths_raw;
            if (auto res = igraph_vector_int_list_init(&paths_raw, 1); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_get_shortest_paths(graph->get_graph(), &paths_raw, nullptr, from_vertex, v_sel, mode, nullptr, nullptr); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_list_destroy(&paths_raw);
                return ERR(igraph_strerror(res));
            }

            std::vector<std::vector<u32>> paths;
            for (u32 i = 0; i < igraph_vector_int_list_size(&paths_raw); i++)
            {
                auto vec = igraph_vector_int_list_get_ptr(&paths_raw, i);

                u32 vec_size = igraph_vector_int_size(vec);
                std::vector<u32> tmp(vec_size);
                for (u32 j = 0; j < igraph_vector_int_size(vec); j++)
                {
                    tmp[j] = VECTOR(*vec)[j];
                }
                paths.push_back(std::move(tmp));
            }

            igraph_vs_destroy(&v_sel);
            igraph_vector_int_list_destroy(&paths_raw);

            return OK(paths);
        }

        Result<std::vector<std::vector<u32>>> get_all_shortest_paths(NetlistGraph* graph, Gate* from_gate, const std::vector<Gate*>& to_gates, NetlistGraph::Direction direction)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (!from_gate)
            {
                return ERR("no source gate provided");
            }

            if (to_gates.empty())
            {
                return ERR("no destination gates provided");
            }

            u32 from_vertex;
            if (auto res = graph->get_vertex_from_gate(from_gate); res.is_ok())
            {
                from_vertex = res.get();
            }
            else
            {
                return ERR(res.get_error());
            }

            igraph_vector_int_t i_to_vertices;
            if (auto res = graph->get_vertices_from_gates_igraph(to_gates); res.is_ok())
            {
                i_to_vertices = std::move(res.get());
            }
            else
            {
                return ERR(res.get_error());
            }

            auto res = get_all_shortest_paths_igraph(graph, from_vertex, &i_to_vertices, direction);

            igraph_vector_int_destroy(&i_to_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<std::vector<u32>>> get_all_shortest_paths(NetlistGraph* graph, u32 from_vertex, const std::vector<u32>& to_vertices, NetlistGraph::Direction direction)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (to_vertices.empty())
            {
                return ERR("no destination vertices provided");
            }

            igraph_vector_int_t i_to_vertices;
            if (auto res = igraph_vector_int_init(&i_to_vertices, to_vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < to_vertices.size(); i++)
            {
                VECTOR(i_to_vertices)[i] = to_vertices.at(i);
            }

            auto res = get_all_shortest_paths_igraph(graph, from_vertex, &i_to_vertices, direction);

            igraph_vector_int_destroy(&i_to_vertices);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::vector<std::vector<u32>>> get_all_shortest_paths_igraph(NetlistGraph* graph, u32 from_vertex, const igraph_vector_int_t* to_vertices, NetlistGraph::Direction direction)
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
            }

            igraph_vs_t v_sel = igraph_vss_vector(to_vertices);
            igraph_vector_int_list_t paths_raw;
            if (auto res = igraph_vector_int_list_init(&paths_raw, 1); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            if (auto res = igraph_get_all_shortest_paths(graph->get_graph(), &paths_raw, nullptr, nullptr, from_vertex, v_sel, mode); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_list_destroy(&paths_raw);
                return ERR(igraph_strerror(res));
            }

            std::vector<std::vector<u32>> paths;
            const u32 paths_size = igraph_vector_int_list_size(&paths_raw);
            for (u32 i = 0; i < paths_size; i++)
            {
                auto vec = igraph_vector_int_list_get_ptr(&paths_raw, i);

                const u32 vec_size = igraph_vector_int_size(vec);
                std::vector<u32> tmp(vec_size);
                for (u32 j = 0; j < vec_size; j++)
                {
                    tmp[j] = VECTOR(*vec)[j];
                }
                paths.push_back(std::move(tmp));
            }

            igraph_vs_destroy(&v_sel);
            igraph_vector_int_list_destroy(&paths_raw);

            return OK(paths);
        }
    }    // namespace graph_algorithm
}    // namespace hal