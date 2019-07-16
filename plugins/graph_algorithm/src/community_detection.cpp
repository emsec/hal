#include "plugin_graph_algorithm.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <boost/graph/graphviz.hpp>
#include <boost/graph/strong_components.hpp>
#include <igraph/igraph.h>

std::set<std::set<std::shared_ptr<gate>>> plugin_graph_algorithm::get_communities(std::shared_ptr<netlist> nl, std::set<std::shared_ptr<gate>> gates)
{
    if (nl == nullptr)
    {
        log_error(this->get_name(), "{}", "parameter 'g' is nullptr");
        return std::set<std::set<std::shared_ptr<gate>>>();
    }
    /* check validity of gates */
    if (gates.empty())
    {
        gates = nl->get_gates();
    }
    for (const auto& current_gate : gates)
    {
        if (current_gate != nullptr)
            continue;
        log_error(this->get_name(), "{}", "parameter 'gates' contains a nullptr");
        return std::set<std::set<std::shared_ptr<gate>>>();
    }


    igraph_t graph;
    igraph_vector_t netlist_edges;

    igraph_empty(&graph, nl->get_gates().size(), IGRAPH_UNDIRECTED);

    // count amount of nets
    // we have to iterate over all edges, since counting the amount of nets in the set
    // doesnt suffice, since nets can have multiple dsts
    u32 net_counter = 0;
    for (auto net : nl->get_nets()) {
        std::shared_ptr<gate> predecessor = net->get_src().get_gate();
        if (predecessor == nullptr)
            continue;

        std::vector<endpoint> successors = net->get_dsts();
        for (auto successor : successors) {
            if (successor.get_gate() == nullptr)
                continue;
            net_counter++;
        }
    }

    log("Net amount: {}", net_counter);
    igraph_real_t edges[net_counter*2];

    int i = 0;
    for (auto net : nl->get_nets()) {
        std::shared_ptr<gate> predecessor = net->get_src().get_gate();
        if (predecessor == nullptr)
            continue;
        u32 predecssor_id = predecessor->get_id() - 1;

        std::vector<endpoint> successors = net->get_dsts();
        for (auto successor : successors) {
            if (successor.get_gate() == nullptr)
                continue;
            u32 successor_id = successor.get_gate()->get_id() - 1;
            edges[i++] = (igraph_real_t)predecssor_id;
            edges[i++] = (igraph_real_t)successor_id;
        }
    }

    igraph_vector_init_copy(&netlist_edges, edges, net_counter*2);
    igraph_add_edges(&graph, &netlist_edges, 0);

    // simplify graph by removing double edges
    igraph_simplify(&graph, true, false, 0);

    log("Edges:     Original {} vs. {}", net_counter, (int)igraph_ecount(&graph));
    log("Vertices:  Original {} vs. {}", nl->get_gates().size(), (int)igraph_vcount(&graph));

    // generate dot file for graph plotting
    plot_graph(&graph);

    igraph_vector_t membership, modularity;
    igraph_vector_init(&membership,1);
    igraph_vector_init(&modularity,1);

    igraph_matrix_t merges;
    igraph_matrix_init(&merges, 1, 1);

    
    igraph_community_fastgreedy(&graph, NULL, &merges, &modularity, &membership);


    igraph_vector_destroy(&netlist_edges);
    igraph_vector_destroy(&membership);
    igraph_vector_destroy(&modularity);
    igraph_matrix_destroy(&merges);
    igraph_destroy(&graph);

    return std::set<std::set<std::shared_ptr<gate>>>();
}

void plugin_graph_algorithm::plot_graph(igraph_t *graph) {

    igraph_vector_t graph_edges;
    igraph_vector_init(&graph_edges, (int)igraph_ecount(graph));
    igraph_get_edgelist(graph, &graph_edges, false);

    std::ofstream ofs("/home/st/hal/graph.dot", std::ofstream::out);

    ofs << "graph {";

     std::string edge;
    for(int i = 0; i < (int)igraph_ecount(graph); i++) {
        edge = "\n    \"" + std::to_string((int)(igraph_vector_e(&graph_edges, i))) + "\" -- \"";
        i++;
        edge += std::to_string((int)(igraph_vector_e(&graph_edges, i))) + "\";";
        ofs << edge;
    }

    ofs << "\n}";

    ofs.close();
    igraph_vector_destroy(&graph_edges);

}