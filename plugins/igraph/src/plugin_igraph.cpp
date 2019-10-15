#include "plugin_igraph.h"

#include <igraph/igraph.h>

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

std::string plugin_igraph::get_name()
{
    return std::string("igraph");
}

std::string plugin_igraph::get_version()
{
    return std::string("0.1");
}

std::set<interface_type> plugin_igraph::get_type()
{
    return {interface_type::base};
}

igraph_t plugin_igraph::get_igraph_directed(std::shared_ptr<netlist> nl)
{
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
    for (const auto& net : nl->get_nets())
    {
        if (net->get_src().get_gate() == nullptr)
            continue;

        for (const auto& successor : net->get_dsts())
        {
            if (successor.get_gate() == nullptr)
                continue;
            edge_counter += 2;
        }
    }

    log("i am counting {} edges", edge_counter);

    /* transform all nets to igraph_real_t */
    igraph_real_t* edges     = new igraph_real_t[edge_counter];
    u32 edge_vertice_counter = 0;
    for (const auto& net : nl->get_nets())
    {
        auto predecessor = net->get_src().get_gate();
        if (predecessor == nullptr)
            continue;
        auto predecessor_id = nl_igraph_id_match[predecessor->get_id()];

        for (const auto& successor : net->get_dsts())
        {
            if (successor.get_gate() == nullptr)
                continue;
            auto successor_id             = nl_igraph_id_match[successor.get_gate()->get_id()];
            edges[edge_vertice_counter++] = (igraph_real_t)predecessor_id;
            edges[edge_vertice_counter++] = (igraph_real_t)successor_id;
        }
    }

    /* create and add edges to the graph */
    igraph_t graph;
    igraph_vector_t netlist_edges;
    igraph_vector_init_copy(&netlist_edges, edges, edge_counter);
    delete[] edges;
    igraph_create(&graph, &netlist_edges, nl->get_gates().size(), IGRAPH_DIRECTED);
    igraph_vector_destroy(&netlist_edges);

    return graph;
}