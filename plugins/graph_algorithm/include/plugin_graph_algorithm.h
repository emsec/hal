#ifndef __HAL_PLUGIN_GRAPH_ALGORITHM_H__
#define __HAL_PLUGIN_GRAPH_ALGORITHM_H__

#include "core/interface_base.h"

#include <hal_bdd.h>

#include <igraph/igraph.h>

/* forward declaration */
class netlist;
class gate;
class net;

class PLUGIN_API plugin_graph_algorithm : public i_base
{
public:
    /** constructor (= default) */
    plugin_graph_algorithm() = default;

    /** destructor (= default) */
    ~plugin_graph_algorithm() = default;

    /*
     *      interface implementations
     */

    /** interface implementation: i_base */
    std::string get_name() override;

    /** interface implementation: i_base */
    std::string get_version() override;

    /*
     *      plugin specific functions
     */


    /**
     * Returns communities
     *
     * @param[in] nl - Netlist (internally transformed to di-graph)
     * @param[in] gates - Set of gates for which the strongly connected components are determined (default = empty means that all gates of the netlist are considered)
     * @returns A community
     */
    std::set<std::set<std::shared_ptr<gate>>> get_communities(std::shared_ptr<netlist> const nl, const std::set<std::shared_ptr<gate>> gates = {});
    void plot_graph(igraph_t *graph);

    /**
     * Returns the set of strongly connected components.
     *
     * @param[in] nl - Netlist (internally transformed to di-graph)
     * @param[in] gates - Set of gates for which the strongly connected components are determined (default = empty means that all gates of the netlist are considered)
     * @returns A set of strongly connected components where each component is a set of gates.
     */
    std::set<std::set<std::shared_ptr<gate>>> get_strongly_connected_components(std::shared_ptr<netlist> const nl, const std::set<std::shared_ptr<gate>> gates = {});

    /**
     * Returns the shortest path distances for one gate to all other gates.
     *
     * @param[in] g - Gate (starting vertex for Dijkstra's algorithm)
     * @returns A map of path and distance to the starting gate g for all other gates in the netlist.
     */
    std::map<std::shared_ptr<gate>, std::tuple<std::vector<std::shared_ptr<gate>>, int>> get_dijkstra_shortest_paths(const std::shared_ptr<gate> g);

    /**
     * Returns the BDD representation for a part of netlist.
     *
     * @param[in] nl - Netlist
     * @param[in] gates - Set of gates defining the submodule for which the BDDs are determined
     * @param[in] input_net_to_ids - Map of submodule input net ids (variable definition X0, X1, ...), if empty the function determines the variable assignment and returns it.
     * @returns A tuple of a map of submodule input net to ids (variable definition X0, X1, ...) and a map of submodule output net to BDDs (function defintion f1(x0, ..., x_n), f2(x0, ... x_n), ...).
     */
    std::tuple<std::map<std::shared_ptr<net>, int>, std::map<std::shared_ptr<net>, std::shared_ptr<bdd>>>
        get_bdds(std::shared_ptr<netlist> const nl,
                           const std::set<std::shared_ptr<gate>> gates,
                           const std::map<std::shared_ptr<net>, int> input_net_to_ids = std::map<std::shared_ptr<net>, int>());

    /**
     * Returns a graph cut for a specific gate and depth.
     *
     * @param[in] nl - Netlist (internally transformed to di-graph)
     * @param[in] g - Gate (starting vertex for graph cut)
     * @param[in] depth - Graph cut depth
     * @param[in] synchronous_gate_type - Marks terminal vertex gate types of graph cut (typically memory gates such as flip-flops)
     * @returns A vector of gate sets where each vector entry refers to the distance to the starting gate g.
     */
    std::vector<std::set<std::shared_ptr<gate>>> get_graph_cut(std::shared_ptr<netlist> const nl,
                                                               std::shared_ptr<gate> const g,
                                                               const u32 depth                                = std::numeric_limits<u32>::max(),
                                                               const std::set<std::string> terminal_gate_type = std::set<std::string>());
};

#endif /* __HAL_PLUGIN_GRAPH_ALGORITHM_H__ */
