#include "graph_algorithm/algorithms/components.h"

#include "graph_algorithm/netlist_graph.h"

#include <map>

namespace hal
{
    namespace graph_algorithm
    {
        Result<std::vector<std::vector<u32>>> get_connected_components(NetlistGraph* graph, bool strong, u32 min_size)
        {
            if (graph == nullptr)
            {
                return ERR("graph is a nullptr");
            }

            igraph_vector_int_t membership, csize;
            igraph_integer_t number_of_clusters;
            auto err = igraph_vector_int_init(&membership, 0);
            if (err != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&membership);
                return ERR(igraph_strerror(err));
            }

            err = igraph_vector_int_init(&csize, 0);
            if (err != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&membership);
                igraph_vector_int_destroy(&csize);
                return ERR(igraph_strerror(err));
            }

            igraph_t* igr = graph->get_graph();

            // run scc
            err = igraph_clusters(igr, &membership, &csize, &number_of_clusters, strong ? IGRAPH_STRONG : IGRAPH_WEAK);
            if (err != IGRAPH_SUCCESS)
            {
                igraph_vector_int_destroy(&membership);
                igraph_vector_int_destroy(&csize);
                return ERR(igraph_strerror(err));
            }

            // map back to HAL structures
            u32 num_vertices = (u32)igraph_vcount(igr);
            std::map<u32, std::set<u32>> components_raw;

            for (i32 i = 0; i < num_vertices; i++)
            {
                components_raw[VECTOR(membership)[i]].insert(i);
            }

            // convert to set
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
            igraph_vector_int_destroy(&csize);

            return OK(components);
        }
    }    // namespace graph_algorithm
}    // namespace hal