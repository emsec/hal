#include "plugin_graph_algorithm.h"

#include "core/log.h"
#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include <tuple>

#include <igraph/igraph.h>

std::map<int, std::set<std::shared_ptr<gate>>> plugin_graph_algorithm::get_communities_fast_greedy(std::shared_ptr<netlist> nl)
{
    if (nl == nullptr)
    {
        log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
        return std::map<int, std::set<std::shared_ptr<gate>>>();
    }

    std::tuple<igraph_t, std::map<int, std::shared_ptr<gate>>> igraph_tuple = get_igraph_directed(nl);
    igraph_t graph                                                          = std::get<0>(igraph_tuple);
    std::map<int, std::shared_ptr<gate>> vertex_to_gate                     = std::get<1>(igraph_tuple);

    igraph_vector_t membership, modularity;
    igraph_matrix_t merges;

    igraph_vector_init(&membership, 1);
    igraph_vector_init(&modularity, 1);
    igraph_matrix_init(&merges, 1, 1);

    igraph_community_fastgreedy(&graph,
                                0, /* no weights */
                                &merges,
                                &modularity,
                                &membership);

    // map back to HAL structures
    std::map<int, std::set<std::shared_ptr<gate>>> community_sets;
    for (int i = 0; i < igraph_vector_size(&membership); i++)
    {
        community_sets[(int)VECTOR(membership)[i]].insert(vertex_to_gate[i]);
    }
    igraph_vector_destroy(&membership);

    igraph_destroy(&graph);
    igraph_vector_destroy(&membership);
    igraph_vector_destroy(&modularity);
    igraph_matrix_destroy(&merges);

    return community_sets;
}
