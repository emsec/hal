#include "plugin_graph_algorithm.h"

#include "core/log.h"
#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include <tuple>

#include <igraph/igraph.h>

std::map<int, std::set<std::shared_ptr<gate>>> plugin_graph_algorithm::get_communities_spinglass(std::shared_ptr<netlist> nl, u32 spins)
{
    if (nl == nullptr)
    {
        log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
        return std::map<int, std::set<std::shared_ptr<gate>>>();
    }

    std::tuple<igraph_t, std::map<int, std::shared_ptr<gate>>> igraph_tuple = get_igraph_directed(nl);
    igraph_t graph                                                          = std::get<0>(igraph_tuple);
    std::map<int, std::shared_ptr<gate>> vertex_to_gate                    = std::get<1>(igraph_tuple);

    igraph_real_t modularity, temperature;
    igraph_vector_t membership, csize;


    igraph_vector_init(&membership, 0);
    igraph_vector_init(&csize, 0);
    igraph_community_spinglass(&graph,
                               0, /* no weights */
                               &modularity,
                               &temperature,
                               &membership,
                               &csize,
                               (igraph_integer_t)spins, /* no of spins */
                               0,                       /* parallel update */
                               1.0,                     /* start temperature */
                               0.01,                    /* stop temperature */
                               0.99,                    /* cooling factor */
                               IGRAPH_SPINCOMM_UPDATE_CONFIG,
                               1.0, /* gamma */
                               IGRAPH_SPINCOMM_IMP_ORIG,
                               /*gamma-=*/0);

    log("Modularity:  {}", modularity);
    log("Temperature: {}", temperature);
    log("Cluster sizes: ");
    for (long int i = 0; i < igraph_vector_size(&csize); i++)
    {
        log("{}", (long int)VECTOR(csize)[i]);
    }
    log("\n");

    // map back to HAL structures
    int vertices_num = (int)igraph_vcount(&graph);
    std::map<int, std::set<std::shared_ptr<gate>>> community_sets = get_memberships_for_hal(graph, membership, vertex_to_gate);

    igraph_destroy(&graph);
    igraph_vector_destroy(&membership);
    igraph_vector_destroy(&csize);

    return community_sets;
}
