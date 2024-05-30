#include "graph_algorithm/algorithms/components.h"

#include "graph_algorithm/netlist_graph.h"

#include <map>

namespace hal
{
    namespace graph_algorithm
    {
        Result<std::vector<std::vector<u32>>> get_connected_components(const NetlistGraph* graph, bool strong, u32 min_size)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            igraph_vector_int_t membership;
            if (auto res = igraph_vector_int_init(&membership, 0); res != IGRAPH_SUCCESS)
            {
                return ERR(igraph_strerror(res));
            }

            const igraph_t* i_graph = graph->get_graph();
            if (auto res = igraph_connected_components(i_graph, &membership, nullptr, nullptr, strong ? IGRAPH_STRONG : IGRAPH_WEAK); res != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&membership);
                return ERR(igraph_strerror(res));
            }

            u32 num_vertices = igraph_vcount(i_graph);
            std::map<u32, std::set<u32>> components_raw;

            for (i32 i = 0; i < num_vertices; i++)
            {
                components_raw[VECTOR(membership)[i]].insert(i);
            }

            std::vector<std::vector<u32>> components;
            for (auto& [_, members] : components_raw)
            {
                if (members.size() < min_size)
                {
                    continue;
                }

                components.push_back(std::vector<u32>(members.begin(), members.end()));
            }

            igraph_vector_int_destroy(&membership);

            return OK(components);
        }
    }    // namespace graph_algorithm
}    // namespace hal