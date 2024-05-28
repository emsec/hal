#include "graph_algorithm/algorithms/subgraph.h"

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/netlist/gate.h"

namespace hal
{
    namespace graph_algorithm
    {
        Result<std::unique_ptr<NetlistGraph>> get_subgraph(NetlistGraph* graph, const std::vector<Gate*>& subgraph_gates)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (subgraph_gates.empty())
            {
                return ERR("no subgraph gates provided");
            }

            igraph_vector_int_t i_gates;
            if (auto res = graph->get_vertices_from_gates_igraph(subgraph_gates); res.is_ok())
            {
                i_gates = std::move(res.get());
            }
            else
            {
                return ERR(res.get_error());
            }

            auto res = get_subgraph_igraph(graph, &i_gates);

            igraph_vector_int_destroy(&i_gates);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::unique_ptr<NetlistGraph>> get_subgraph(NetlistGraph* graph, const std::vector<u32>& subgraph_vertices)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            if (subgraph_vertices.empty())
            {
                return ERR("no subgraph vertices provided");
            }

            igraph_vector_int_t i_gates;
            if (auto res = igraph_vector_int_init(&i_gates, subgraph_vertices.size()); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            for (u32 i = 0; i < subgraph_vertices.size(); i++)
            {
                VECTOR(i_gates)[i] = subgraph_vertices.at(i);
            }

            auto res = get_subgraph_igraph(graph, &i_gates);

            igraph_vector_int_destroy(&i_gates);

            if (res.is_error())
            {
                return ERR(res.get_error());
            }

            return res;
        }

        Result<std::unique_ptr<NetlistGraph>> get_subgraph_igraph(NetlistGraph* graph, const igraph_vector_int_t* subgraph_vertices)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            igraph_vs_t v_sel = igraph_vss_vector(subgraph_vertices);
            u32 subgraph_size = igraph_vector_int_size(subgraph_vertices);

            igraph_vector_int_t i_vertex_map;
            if (auto res = igraph_vector_int_init(&i_vertex_map, subgraph_size); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                return ERR(igraph_strerror(res));
            }

            igraph_t i_subg;
            if (const auto res = igraph_induced_subgraph_map(graph->get_graph(), &i_subg, v_sel, IGRAPH_SUBGRAPH_AUTO, nullptr, &i_vertex_map); res != IGRAPH_SUCCESS)
            {
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_destroy(&i_vertex_map);
                return ERR(igraph_strerror(res));
            }

            std::unordered_map<u32, Gate*> nodes_to_gates;
            if (const auto res = graph->get_gates_from_vertices_igraph(&i_vertex_map); res.is_ok())
            {
                std::vector<Gate*> gates = res.get();
                for (u32 i = 0; i < gates.size(); i++)
                {
                    nodes_to_gates[i] = gates.at(i);
                }
            }
            else
            {
                igraph_destroy(&i_subg);
                igraph_vs_destroy(&v_sel);
                igraph_vector_int_destroy(&i_vertex_map);
                return ERR(res.get_error());
            }

            auto subgraph = std::unique_ptr<NetlistGraph>(new NetlistGraph(graph->get_netlist(), std::move(i_subg), std::move(nodes_to_gates)));

            igraph_vs_destroy(&v_sel);
            igraph_vector_int_destroy(&i_vertex_map);

            return OK(std::move(subgraph));
        }
    }    // namespace graph_algorithm
}    // namespace hal