#include "plugin_graph_algorithm.h"

#include "core/log.h"
#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <igraph/igraph.h>

#include <stdio.h>

std::set<std::set<std::shared_ptr<gate>>> plugin_graph_algorithm::get_scc(std::shared_ptr<netlist> nl)
{
    if (nl == nullptr)
    {
        log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
        return std::set<std::set<std::shared_ptr<gate>>>();
    }

    // get igraph
    std::tuple<igraph_t, std::map<int, std::shared_ptr<gate>>> igraph_tuple = get_igraph_directed(nl);
    igraph_t graph = std::get<0>(igraph_tuple);
    std::map<int, std::shared_ptr<gate>> vertex_to_gate = std::get<1>(igraph_tuple);

    igraph_vector_t membership, csize;
    igraph_integer_t number_of_clusters;
    igraph_vector_init(&membership, 0);
    igraph_vector_init(&csize, 0);

    // run scc
    igraph_clusters(&graph, &membership, &csize, &number_of_clusters, IGRAPH_STRONG);

    // map back to HAL structures
    std::map<int, std::set<std::shared_ptr<gate>>> ssc_membership = get_memberships_for_hal(graph, membership, vertex_to_gate);

    // convert to set
    std::set<std::set<std::shared_ptr<gate>>> sccs;
    for (auto scc : ssc_membership)
    {
        sccs.insert(scc.second);
    }
    return sccs;
}
