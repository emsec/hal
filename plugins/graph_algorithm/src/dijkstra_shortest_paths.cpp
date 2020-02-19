#include "plugin_graph_algorithm.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>

std::map<std::shared_ptr<gate>, std::tuple<std::vector<std::shared_ptr<gate>>, int>> plugin_graph_algorithm::get_dijkstra_shortest_paths(const std::shared_ptr<gate> g)
{
    if (g == nullptr)
    {
        log_error(this->get_name(), "parameter 'g' is nullptr");
        return {};
    }

    /*
     * boost graph definition for a directed graph
     *
     * boost::adjacency_list - adjacency list
     * boost::listS - stores edge set of each vertex in a list
     * boost::vecS - stores the vertex set in a vector
     * boost::directedS - graph is directed
     * boost::no_property - no information stored to vertex
     * boost::property<boost::edge_weight_t, int> - weight information stored to edge
     */
    typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property, boost::property<boost::edge_weight_t, int>> boost_graph_t;

    /*
     * boost vertex definition for the boost_graph_t
     */
    typedef boost::graph_traits<boost_graph_t>::vertex_descriptor vertex_t;

    boost_graph_t boost_graph;

    // add gates ordered by id (for determinisitc behavior) to boost graph
    // vertices in boost graph are ordered from 0, 1, ...
    auto nl = g->get_netlist();
    std::set<u32> gate_ids;
    for (const auto& gate : nl->get_gates())
        gate_ids.insert(gate->get_id());

    std::map<u32, vertex_t> gate_id_to_vertex;
    std::map<vertex_t, u32> vertex_id_to_gate_id;

    u32 vertex_id_cnt = 0;
    for (const auto& gate_id : gate_ids)
    {
        auto vd                               = boost::add_vertex(boost_graph);
        gate_id_to_vertex[gate_id]            = vd;
        vertex_id_to_gate_id[vertex_id_cnt++] = gate_id;
    }

    // add ordered weigthened edges (weight = 1) to directed boost graph
    std::map<u32, std::shared_ptr<net>> ordered_nets;
    for (const auto& net : nl->get_nets())
        ordered_nets[net->get_id()] = net;

    for (const auto& it : ordered_nets)
    {
        if (it.second->get_src().get_gate() == nullptr)
            continue;

        std::set<u32> dst_ids;
        for (auto dst : it.second->get_dsts())
            dst_ids.insert(dst.get_gate()->get_id());

        for (const auto& dst_id : dst_ids)
            boost::add_edge(gate_id_to_vertex[it.second->get_src().get_gate()->get_id()], gate_id_to_vertex[dst_id], 1, boost_graph);
    }

    // initialize parameters for dijkstra_shortest_paths()
    std::vector<vertex_t> predecessors(nl->get_gates().size());
    std::vector<int> distance(nl->get_gates().size());

    dijkstra_shortest_paths(boost_graph,
                            gate_id_to_vertex[g->get_id()],
                            predecessor_map(boost::make_iterator_property_map(predecessors.begin(), get(boost::vertex_index, boost_graph)))
                                .distance_map(boost::make_iterator_property_map(distance.begin(), get(boost::vertex_index, boost_graph))));

    // postprocess boost result
    std::map<std::shared_ptr<gate>, std::tuple<std::vector<std::shared_ptr<gate>>, int>> result;
    boost::graph_traits<boost_graph_t>::vertex_iterator vi, vend;
    for (boost::tie(vi, vend) = boost::vertices(boost_graph); vi != vend; ++vi)
    {
        if (distance[*vi] == ((i64)1 << 31) - 1)
        {
            // no path from g to gate
            result[nl->get_gate_by_id(vertex_id_to_gate_id[*vi])] = std::make_tuple(std::vector<std::shared_ptr<gate>>(), -1);
        }
        else
        {
            // path from src to gate, so assemble path
            std::vector<std::shared_ptr<gate>> path;
            auto tmp = *vi;
            while (vertex_id_to_gate_id[tmp] != g->get_id())
            {
                path.push_back(nl->get_gate_by_id(vertex_id_to_gate_id[tmp]));
                tmp = predecessors[tmp];
            }
            path.push_back(g);
            std::reverse(path.begin(), path.end());
            result[nl->get_gate_by_id(vertex_id_to_gate_id[*vi])] = std::make_tuple(path, distance[*vi]);
        }
    }
    return result;
}
