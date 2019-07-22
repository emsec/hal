#include "plugin_graph_algorithm.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <fstream>
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

    /* delete leaves */
    bool deleted_leave;
    do {
        deleted_leave = false;
        for (const auto temp_gate : nl->get_gates()) {
            u32 counter = 0;
            for (const auto pred : temp_gate->get_predecessors()) {
                counter++;
            }
            for (const auto suc : temp_gate->get_successors()) {
                counter++;
            }
            if (counter < 2) {
                nl->delete_gate(temp_gate);
                deleted_leave = true;
            }
            /* delete leaves connected to a single gate as successor and predecessor */
            if ((counter == 2) && (temp_gate->get_predecessors().size() == 1) && (temp_gate->get_successors().size() == 1)) {
                if ((temp_gate->get_predecessors()[0].get_gate() == temp_gate->get_successors()[0].get_gate())) {
                    nl->delete_gate(temp_gate);
                    deleted_leave = true;
                }
            }
        }
    } while(deleted_leave);

    /* save original gate-IDs */
    std::map<int, int> gate_id_match, id_gate_match;
    u32 id_count = 0;
    for (const auto temp_gate : nl->get_gates()) {
        gate_id_match[temp_gate->get_id()] = id_count;
        id_gate_match[id_count] = temp_gate->get_id();
        id_count++;
    }

    igraph_t graph;
    igraph_vector_t netlist_edges;
    igraph_empty(&graph, nl->get_gates().size(), IGRAPH_UNDIRECTED);

    /* count amount of nets, with all destinations of all nets */
    u32 net_counter = 0;
    for (const auto net : nl->get_nets()) {
        std::shared_ptr<gate> predecessor = net->get_src().get_gate();
        if (predecessor == nullptr)
            continue;

        std::vector<endpoint> successors = net->get_dsts();
        for (const auto successor : successors) {
            if (successor.get_gate() == nullptr)
                continue;
            net_counter++;
        }
    }

    /* save all nets in igraph_real_t */
    igraph_real_t edges[net_counter*2];
    u32 edge_counter = 0;
    for (const auto net : nl->get_nets()) {
        std::shared_ptr<gate> predecessor = net->get_src().get_gate();
        if (predecessor == nullptr)
            continue;
        u32 predecessor_id = gate_id_match[predecessor->get_id()];

        std::vector<endpoint> successors = net->get_dsts();
        for (const auto successor : successors) {
            if (successor.get_gate() == nullptr)
                continue;
            u32 successor_id = gate_id_match[successor.get_gate()->get_id()];
            edges[edge_counter++] = (igraph_real_t)predecessor_id;
            edges[edge_counter++] = (igraph_real_t)successor_id;
        }
    }

    /* add edges to the graph and remove double edges */
    igraph_vector_init_copy(&netlist_edges, edges, net_counter*2);
    igraph_add_edges(&graph, &netlist_edges, 0);
    igraph_simplify(&graph, true, false, 0);

    igraph_vector_t membership, modularity;
    igraph_vector_init(&membership,1);
    igraph_vector_init(&modularity,1);
    igraph_matrix_t merges;
    igraph_matrix_init(&merges, 1, 1);

    /* Louvain method without weights */
    igraph_community_fastgreedy(&graph, NULL, &merges, &modularity, &membership);

    show_results(&modularity, &membership, get_community_sets(&membership, nl, id_gate_match), false);
    plot_community_graph(&graph, &membership, id_gate_match);

    igraph_vector_destroy(&netlist_edges);
    igraph_vector_destroy(&membership);
    igraph_vector_destroy(&modularity);
    igraph_matrix_destroy(&merges);
    igraph_destroy(&graph);

    return std::set<std::set<std::shared_ptr<gate>>>(); //get_community_sets(&membership, nl, id_gate_match);
}

std::map<int, std::set<std::shared_ptr<gate>>> plugin_graph_algorithm::get_community_sets(igraph_vector_t *membership, std::shared_ptr<netlist> nl, std::map<int, int> id_gate_match) {
    std::map<int, std::set<std::shared_ptr<gate>>> community_sets;
    
    for (int i = 0; i < igraph_vector_size(membership); i++) {
        community_sets[(long int)VECTOR(*membership)[i]].insert(nl->get_gate_by_id(id_gate_match[i]));
    }

    return community_sets;
}

void plugin_graph_algorithm::show_results(igraph_vector_t *modularity, igraph_vector_t *membership, std::map<int, std::set<std::shared_ptr<gate>>> community_sets, bool show_detailed_communities) {
    long int max = igraph_vector_which_max(modularity);
    log("Modularity:  {}", VECTOR(*modularity)[max]);

    u32 communities = 0;
    for (int i = 0; i < igraph_vector_size(membership); i++) {
        if (communities < (u32)VECTOR(*membership)[i]) {
            communities = (u32)VECTOR(*membership)[i];
        }
    }
    log("Communities: {}", communities+1);

    if (show_detailed_communities){
        for (const auto community_set : community_sets) {
            log("Community {}:", community_set.first);
            for (const auto temp_gate : community_set.second) {
                log("\tID: {}\t\tName: {}", temp_gate->get_id(), temp_gate->get_name());
            }
        }

        std::ofstream ofs("/home/st/hal/community_sets.txt", std::ofstream::out);

        for (const auto community_set : community_sets) {
            ofs << "Community " + std::to_string(community_set.first) + ":\n";
            for (const auto temp_gate : community_set.second) {
                ofs << "\tID: " + std::to_string(temp_gate->get_id()) + "\t\tName: " + temp_gate->get_name() + "\n";
            }
        }
    }
}

void plugin_graph_algorithm::plot_community_graph(igraph_t *graph, igraph_vector_t *membership, std::map<int, int> id_gate_match) {
    /* get edges of graph */
    igraph_vector_t graph_edges;
    igraph_vector_init(&graph_edges, (int)igraph_ecount(graph));
    igraph_get_edgelist(graph, &graph_edges, false);
    
    /* colors for communities */
    std::map<int, std::string> colors = {{0,"aquamarine"}, {1, "bisque"}, {2, "brown"}, {3, "chartreuse"}, {4, "darkorchid"}, {5, "cyan"},
                                         {6, "gold"}, {7, "cornflowerblue"}, {8, "azure4"}, {9, "darkorchid4"}, {10, "forestgreen"}}; 

    std::ofstream ofs("/home/st/hal/graph.dot", std::ofstream::out);

    ofs << "graph {";

    /* color nodes in community colors, currently only 11 different colors */
    if(membership != NULL) {
        for(int i = 0; i < (int)igraph_vcount(graph); i++) {
            ofs << "\n    " + std::to_string(id_gate_match[i]) + "[style=filled fillcolor=\"" + colors[(int)VECTOR(*membership)[i] % 11] + "\"];";
        }
    }

    /* add edges */
    std::string edge;
    for(int i = 0; i < (int)igraph_ecount(graph)*2; i++) {
        edge = "\n    \"" + std::to_string(id_gate_match[(int)(igraph_vector_e(&graph_edges, i))]) + "\" -- \"";
        i++;
        edge += std::to_string(id_gate_match[(int)(igraph_vector_e(&graph_edges, i))]) + "\";";
        ofs << edge;
    }

    ofs << "\n}";

    ofs.close();
    igraph_vector_destroy(&graph_edges);
}