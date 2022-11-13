#include "graph_algorithm/plugin_graph_algorithm.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <igraph.h>

namespace hal
{
    std::map<int, std::set<Gate*>> GraphAlgorithmPlugin::get_communities(Netlist* nl)
    {
        if (nl == nullptr)
        {
            log_error(this->get_name(), "{}", "parameter 'g' is nullptr");
            return std::map<int, std::set<Gate*>>();
        }

        /* delete leaves */
        bool deleted_leave;
        do
        {
            deleted_leave = false;
            for (const auto& test_gate : nl->get_gates())
            {
                u32 counter = test_gate->get_predecessors().size() + test_gate->get_successors().size();
                if (counter < 2)
                {
                    nl->delete_gate(test_gate);
                    deleted_leave = true;
                }
                /* delete leaves connected to a single gate as successor and predecessor */
                else if ((counter == 2) && (test_gate->get_predecessors().size() == 1) && (test_gate->get_successors().size() == 1))
                {
                    if (test_gate->get_predecessors()[0]->get_gate() == test_gate->get_successors()[0]->get_gate())
                    {
                        nl->delete_gate(test_gate);
                        deleted_leave = true;
                    }
                }
            }
        } while (deleted_leave);

        /* map netlist to igraph IDs for both directions */
        std::map<int, int> nl_igraph_id_match, igraph_nl_id_match;
        u32 id_count = 0;
        for (const auto& single_gate : nl->get_gates())
        {
            nl_igraph_id_match[single_gate->get_id()] = id_count;
            igraph_nl_id_match[id_count]              = single_gate->get_id();
            id_count++;
        }

        /* count amount of nets, with all destinations of all nets */
        u32 edge_counter = 0;
        for (auto net : nl->get_nets())
        {
            assert(net->get_sources().size() == 1);
            if (net->get_sources().front()->get_gate() == nullptr)
                continue;

            for (const auto& successor : net->get_destinations())
            {
                if (successor->get_gate() == nullptr)
                    continue;
                edge_counter += 2;
            }
        }

        /* transform all nets to igraph_real_t */
        igraph_integer_t *edges  = new igraph_integer_t[edge_counter];
        u32 edge_vertex_counter = 0;
        for (auto net : nl->get_nets())
        {
            assert(net->get_sources().size() == 1);
            Gate* predecessor = net->get_sources().front()->get_gate();
            if (predecessor == nullptr)
                continue;
            u32 predecessor_id = nl_igraph_id_match[predecessor->get_id()];

            for (const auto& successor : net->get_destinations())
            {
                if (successor->get_gate() == nullptr)
                    continue;
                auto successor_id             = nl_igraph_id_match[successor->get_gate()->get_id()];
                edges[edge_vertex_counter++] = predecessor_id;
                edges[edge_vertex_counter++] = successor_id;
            }
        }

        /* create and add edges to the graph */
        igraph_t graph;
        igraph_vector_int_t netlist_edges;
        igraph_vector_int_init_array(&netlist_edges, edges, edge_counter);
        delete[] edges;
        igraph_create(&graph, &netlist_edges, nl->get_gates().size(), IGRAPH_UNDIRECTED);
        igraph_vector_int_destroy(&netlist_edges);

        /* remove multi-edges */
        igraph_simplify(&graph, true, false, nullptr);

        /* Greedy modularity maximization without weights */
        igraph_vector_int_t membership;
        igraph_vector_int_init(&membership, 0);
        igraph_community_fastgreedy(&graph, nullptr, nullptr, nullptr, &membership);
        igraph_destroy(&graph);

        /* group gates by community membership */
        std::map<int, std::set<Gate*>> community_sets;
        for (igraph_integer_t i = 0; i < igraph_vector_int_size(&membership); i++)
        {
            community_sets[VECTOR(membership)[i]].insert(nl->get_gate_by_id(igraph_nl_id_match[i]));
        }
        igraph_vector_int_destroy(&membership);

        return community_sets;
    }
}    // namespace hal
