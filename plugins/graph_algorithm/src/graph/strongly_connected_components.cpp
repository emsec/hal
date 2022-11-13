#include "graph_algorithm/plugin_graph_algorithm.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <igraph.h>

namespace hal
{
    std::vector<std::vector<Gate*>> GraphAlgorithmPlugin::get_strongly_connected_components(Netlist* nl)
    {
        if (nl == nullptr)
        {
            log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
            return std::vector<std::vector<Gate*>>();
        }

        // get igraph
        igraph_t graph;
        std::map<int, Gate*> vertex_to_gate = get_igraph_directed(nl, &graph);

        igraph_vector_int_t membership;
        igraph_integer_t number_of_clusters;
        igraph_vector_int_init(&membership, 0);

        // run scc
        igraph_connected_components(&graph, &membership, nullptr, &number_of_clusters, IGRAPH_STRONG);

        // map back to HAL structures
        std::map<int, std::set<Gate*>> ssc_membership = get_memberships_for_hal(&graph, &membership, vertex_to_gate);

        // convert to set
        std::vector<std::vector<Gate*>> sccs;
        for (const auto& scc : ssc_membership)
        {
            std::vector<Gate*> scc_vector;
            for (const auto& scc_gate : scc.second)
            {
                scc_vector.push_back(scc_gate);
            }
            sccs.push_back(scc_vector);
        }

        // cleanup
        igraph_destroy(&graph);
        igraph_vector_int_destroy(&membership);

        return sccs;
    }
}    // namespace hal
