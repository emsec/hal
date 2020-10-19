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
    std::map<int, std::set<Gate*>> GraphAlgorithmPlugin::get_communities_spinglass(Netlist* const nl, u32 const spins)
    {
        if (nl == nullptr)
        {
            log_error(this->get_name(), "{}", "parameter 'nl' is nullptr");
            return std::map<int, std::set<Gate*>>();
        }

        log_info("graph_algorithm", "netlist has {} gates and {} nets", nl->get_gates().size(), nl->get_nets().size());

        std::tuple<igraph_t, std::map<int, Gate*>> igraph_tuple = get_igraph_directed(nl);

        igraph_t graph                      = std::get<0>(igraph_tuple);
        std::map<int, Gate*> vertex_to_gate = std::get<1>(igraph_tuple);

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

        log("Clustering successful:");
        log("\tModularity:  {}", modularity);

        log("\tTemperature: {}", temperature);
        log("\tCluster sizes: ");
        for (long int i = 0; i < igraph_vector_size(&csize); i++)
        {
            log("\t\t{}", (long int)VECTOR(csize)[i]);
        }

        // map back to HAL structures
        auto community_sets = get_memberships_for_hal(graph, membership, vertex_to_gate);

        igraph_destroy(&graph);
        igraph_vector_destroy(&membership);
        igraph_vector_destroy(&csize);

        return community_sets;
    }
}    // namespace hal
