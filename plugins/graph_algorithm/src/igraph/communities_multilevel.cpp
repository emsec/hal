#include "graph_algorithm/plugin_graph_algorithm.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"

#include <igraph/igraph.h>
#include <tuple>

namespace hal
{
    std::map<int, std::set<Gate*>> GraphAlgorithmPlugin::get_communities_multilevel(Netlist* nl)
    {
        if (nl == nullptr)
        {
            log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
            return std::map<int, std::set<Gate*>>();
        }

        std::tuple<igraph_t, std::map<int, Gate*>> igraph_tuple = get_igraph_directed(nl);
        igraph_t graph                                          = std::get<0>(igraph_tuple);
        std::map<int, Gate*> vertex_to_gate                     = std::get<1>(igraph_tuple);

        // convert to undirected
        igraph_to_undirected(&graph, IGRAPH_TO_UNDIRECTED_MUTUAL, 0);

        igraph_vector_t membership_vec, modularity;
        igraph_matrix_t membership_mat;

        igraph_vector_init(&membership_vec, 1);
        igraph_vector_init(&modularity, 1);
        igraph_matrix_init(&membership_mat, 1, 1);

        igraph_community_multilevel(&graph,
                                    0, /* no weights */
                                    &membership_vec,
                                    &membership_mat,
                                    &modularity);

        // map back to HAL structures
        std::map<int, std::set<Gate*>> community_sets;
        for (int i = 0; i < igraph_vector_size(&membership_vec); i++)
        {
            community_sets[(int)VECTOR(membership_vec)[i]].insert(vertex_to_gate[i]);
        }

        igraph_destroy(&graph);
        igraph_vector_destroy(&membership_vec);
        igraph_vector_destroy(&modularity);
        igraph_matrix_destroy(&membership_mat);

        return community_sets;
    }
}    // namespace hal
