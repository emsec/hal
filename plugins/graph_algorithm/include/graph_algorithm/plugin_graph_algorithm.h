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

        /**
         * Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /*
        *      clustering function
        */

        /**
         * Get a map of community IDs to communities. Each community is represented by a set of gates.
         *
         * @param[in] netlist - The netlist to operate on.
         * @returns A map from community IDs to communities.
         */
        std::map<int, std::set<Gate*>> get_communities(Netlist* const netlist);

        /**
         * Get a map of community IDs to communities running the spinglass clustering algorithm. Each community is represented by a set of gates.
         *
         * @param[in] netlist - The netlist to operate on.
         * @param[in] spins - The number of spins.
         * @returns A map from community IDs to communities.
         */
        std::map<int, std::set<Gate*>> get_communities_spinglass(Netlist* const netlist, u32 const spins);

        /**
         * Get a map of community IDs to communities running the fast greedy clustering algorithm from igraph. Each community is represented by a set of gates.
         *
         * @param[in] netlist - The netlist to operate on.
         * @returns A map from community IDs to communities.
         */
        std::map<int, std::set<Gate*>> get_communities_fast_greedy(Netlist* const netlist);

        /**
         * Get a map of community IDs to communities running the multilevel clustering algorithm from igraph. Each community is represented by a set of gates.
         *
         * @param[in] netlist - The netlist to operate on.
         * @returns A map from community IDs to communities.
         */
        std::map<int, std::set<Gate*>> get_communities_multilevel(Netlist* netlist);

        /**
         *  other graph algorithm
         */

        /**
         * Get a vector of strongly connected components (SCC) with each SSC being represented by a vector of gates.
         *
         * @param[in] netlist - The netlist to operate on.
         * @returns A vector of SCCs.
         */
        std::vector<std::vector<Gate*>> get_strongly_connected_components(Netlist* netlist);

        /**
         * Get a graph cut for a specific gate and depth. Further, a set of gates can be specified that limit the graph cut, i.e., flip-flops and memory cells.<br>
         * The graph cut is returned as a vector of sets of gates with the vector's index representing the distance of each set to the starting point.
         *
         * @param[in] netlist - The netlist to operate on.
         * @param[in] gate - The gate that is the starting point for the graph cut.
         * @param[in] depth - The depth of the graph cut.
         * @param[in] terminal_gate_type - A set of gates at which to terminate the graph cut.
         * @returns The graph cut as a vector of sets of gates.
         */
        std::vector<std::set<Gate*>>
            get_graph_cut(Netlist* const netlist, Gate* gate, const u32 depth = std::numeric_limits<u32>::max(), const std::set<std::string> terminal_gate_type = std::set<std::string>());

        /*
         *      igraph specific functions
         */

        /**
         * TODO fixme (and add PyBinds)
         * Return a map of sets of gates with the same membership id
         *
         * @param[in] graph - igraph graph object
         * @param[in] membership - membership vector
         * @param[in] vertex_to_gate - map from vertex ID in igraph to HAL gate
         * @returns map from membership id to set of gates that have the membership.
         */
        std::map<int, std::set<Gate*>> get_memberships_for_hal(igraph_t* graph, igraph_vector_t membership, std::map<int, Gate*> vertex_to_gate);

        /**
         * TODO fixme (and add PyBinds)
         * Return an igraph object for the provided netlist.
         *
         * @param[in] netlist - The netlist to operate on.
         * @returns tuple of igraph_t object and map from igraph vertex id to HAL gate ID for further graph analysis.
         */
        std::map<int, Gate*> get_igraph_directed(Netlist* const netlist, igraph_t* igraph);
    };
}    // namespace hal