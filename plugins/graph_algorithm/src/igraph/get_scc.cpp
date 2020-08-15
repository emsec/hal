#include "core/log.h"
#include "core/plugin_manager.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "plugin_graph_algorithm.h"

#include <igraph/igraph.h>
#include <stdio.h>

namespace hal
{
    std::set<std::set<Gate*>> plugin_graph_algorithm::get_scc(Netlist* nl)
    {
        if (nl == nullptr)
        {
            log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
            return std::set<std::set<Gate*>>();
        }

        // get igraph
        std::tuple<igraph_t, std::map<int, Gate*>> igraph_tuple = get_igraph_directed(nl);
        igraph_t graph                                                          = std::get<0>(igraph_tuple);
        std::map<int, Gate*> vertex_to_gate                     = std::get<1>(igraph_tuple);

        igraph_vector_t membership, csize;
        igraph_integer_t number_of_clusters;
        igraph_vector_init(&membership, 0);
        igraph_vector_init(&csize, 0);

        // run scc
        igraph_clusters(&graph, &membership, &csize, &number_of_clusters, IGRAPH_STRONG);

        // map back to HAL structures
        std::map<int, std::set<Gate*>> ssc_membership = get_memberships_for_hal(graph, membership, vertex_to_gate);

        // convert to set
        std::set<std::set<Gate*>> sccs;
        for (auto scc : ssc_membership)
        {
            sccs.insert(scc.second);
        }
        return sccs;
    }
}    // namespace hal
