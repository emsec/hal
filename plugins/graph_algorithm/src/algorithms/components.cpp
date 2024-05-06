#include "graph_algorithm/algorithms/components.h"

#include "graph_algorithm/netlist_graph.h"

#include <map>

namespace hal
{
    namespace graph_algorithm
    {
        Result<std::set<std::set<u32>>> get_connected_components(const NetlistGraph* graph, bool strong)
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
            std::map<u32, std::set<u32>> components;

            for (i32 i = 0; i < num_vertices; i++)
            {
                components[VECTOR(membership)[i]].insert(i);
            }

            // convert to set
            std::set<std::set<u32>> sccs;
            for (auto& [_, members] : components)
            {
                sccs.insert(std::move(members));
            }

            igraph_vector_int_destroy(&membership);
            igraph_vector_int_destroy(&csize);

            return OK(sccs);
        }
    }    // namespace graph_algorithm
}    // namespace hal