#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

#include <igraph/igraph.h>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;
    class Net;

    class PLUGIN_API GraphAlgorithmPlugin : public BasePluginInterface
    {
    public:
        /** constructor (= default) */
        GraphAlgorithmPlugin() = default;

        /** destructor (= default) */
        ~GraphAlgorithmPlugin() = default;

        /*
        *      interface implementations
        */

        /** interface implementation: BasePluginInterface */
        std::string get_name() const override;

        /** interface implementation: BasePluginInterface */
        std::string get_version() const override;

        /*
        *      clustering function
        */

        /**
         * Returns map of community-IDs to communities.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @returns A map of community-IDs to sets of gates belonging to the communities
         */
        std::map<int, std::set<Gate*>> get_communities(Netlist* const nl);

        /**
         * Returns map of community-IDs to communities running the spinglass clustering.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @param[in] spin -
         * @returns A map of community-IDs to sets of gates belonging to the communities
         */
        std::map<int, std::set<Gate*>> get_communities_spinglass(Netlist* const nl, u32 const spins);

        /**
         * Returns map of community-IDs to communities running the fast greedy clustering from igraph.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @param[in] spin -
         * @returns A map of community-IDs to sets of gates belonging to the communities
         */
        std::map<int, std::set<Gate*>> get_communities_fast_greedy(Netlist* const nl);

        /**
         * Returns map of community-IDs to communities running the multilevel clustering from igraph.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @param[in] spin -
         * @returns A map of community-IDs to sets of gates belonging to the communities
         */
        std::map<int, std::set<Gate*>> get_communities_multilevel(Netlist* nl);

        /**
         *  other graph algorithm
         */

        /**
         * Returns the set of strongly connected components.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @param[in] gates - Set of gates for which the strongly connected components are determined (default = empty means that all gates of the netlist are considered)
         * @returns A set of strongly connected components where each component is a set of gates.
         */
        std::vector<std::vector<Gate*>> get_strongly_connected_components(Netlist* const nl, const std::vector<Gate*> gates = {});

        /**
         * Returns the set of strongly connected components.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @param[in] gates - Set of gates for which the strongly connected components are determined (default = empty means that all gates of the netlist are considered)
         * @returns A set of strongly connected components where each component is a set of gates.
         */
        std::set<std::set<Gate*>> get_scc(Netlist* nl);

        /**
         * Returns the shortest path distances for one gate to all other gates.
         *
         * @param[in] g - Gate (starting vertex for Dijkstra's algorithm)
         * @returns A map of path and distance to the starting gate g for all other gates in the netlist.
         */
        std::map<Gate*, std::tuple<std::vector<Gate*>, int>> get_dijkstra_shortest_paths(Gate* g);

        /**
         * Returns a graph cut for a specific gate and depth.
         *
         * @param[in] nl - Netlist (internally transformed to di-graph)
         * @param[in] g - Gate (starting vertex for graph cut)
         * @param[in] depth - Graph cut depth
         * @param[in] synchronous_gate_type - Marks terminal vertex gate types of graph cut (typically memory gates such as flip-flops)
         * @returns A vector of gate sets where each vector entry refers to the distance to the starting gate g.
         */
        std::vector<std::set<Gate*>> get_graph_cut(Netlist* const nl,
                                                                   Gate* g,
                                                                   const u32 depth                                = std::numeric_limits<u32>::max(),
                                                                   const std::set<std::string> terminal_gate_type = std::set<std::string>());

        /*
         *      igraph specific functions
         */

        /**
         * Return a map of sets of gates with the same membership id
         *
         * @param[in] graph - igraph graph object
         * @param[in] membership - membership vector
         * @param[in] vertex_to_gate - map from vertex ID in igraph to HAL gate
         * @returns map from membership id to set of gates that have the membership.
         */
        std::map<int, std::set<Gate*>> get_memberships_for_hal(igraph_t graph, igraph_vector_t membership, std::map<int, Gate*> vertex_to_gate);

        /**
         * Return the igraph object from the provided netlist.
         *
         * @param[in] nl - Netlist
         * @returns tuple of igraph_t object and map from igraph vertex id to HAL gate ID for further graph analysis.
         */
        std::tuple<igraph_t, std::map<int, Gate*>> get_igraph_directed(Netlist* const nl);
    };
}    // namespace hal
