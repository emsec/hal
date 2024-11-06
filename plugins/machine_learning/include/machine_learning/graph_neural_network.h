/**
 * @file netlist_graph.h
 * @brief This file contains structures and functions for creating and annotating netlist graphs used in machine learning applications.
 */

#pragma once

#include "hal_core/defines.h"
#include "machine_learning/types.h"

#include <vector>

namespace hal
{
    class Netlist;
    class Gate;

    namespace machine_learning
    {

        /**
         * @struct NetlistGraph
         * @brief Represents a directed graph structure for a netlist.
         *
         * This structure holds the edges of the graph as pairs of node indices and the graph's directional configuration.
         */
        struct NetlistGraph
        {
            /**
             * @brief Edge list of the graph represented by pairs of node indices.
             */
            std::pair<std::vector<u32>, std::vector<u32>> edge_list;

            /**
             * @brief The direction of the graph (e.g., forward or backward).
             */
            GraphDirection direction;
        };

        /**
         * @brief Constructs a graph representation of a netlist. The connections are an edge list of indices representing the position of the gates in the gates vector.
         *
         * This function constructs a netlist graph by analyzing the connections among all gates in a netlist.
         *
         * @param[in] nl - The netlist to operate on.
         * @param[in] gates - The order of the gates, needed for the index representation.
         * @param[in] dir - The direction of the graph.
         * @returns A `NetlistGraph` representing the netlist connections.
         */
        NetlistGraph construct_netlist_graph(const Netlist* nl, const std::vector<Gate*>& gates, const GraphDirection& dir);

        /**
         * @brief Constructs a sequential netlist graph representation. The connections are an edge list of indices representing the position of the gates in the gates vector.
         *
         * This function constructs a sequential graph representation of the netlist, capturing only the sequential dependencies between gates.
         *
         * @param[in] nl - The netlist to operate on.
         * @param[in] gates - The order of the gates, needed for the index representation.
         * @param[in] dir - The direction of the graph.
         * @returns A `NetlistGraph` representing the sequential connections within the netlist.
         */
        NetlistGraph construct_sequential_netlist_graph(const Netlist* nl, const std::vector<Gate*>& gates, const GraphDirection& dir);

        /**
         * @brief Annotates a netlist graph with features for machine learning tasks.
         *
         * This function annotates a netlist graph by attaching features to each node. This is only for visualization and testing purposes.
         *
         * @param[in] nl - The netlist to annotate.
         * @param[in] gates - The set of gates in the netlist graph.
         * @param[in] nlg - The netlist graph to annotate.
         * @param[in] node_features - A matrix of features for each node.
         */
        void annotate_netlist_graph(Netlist* nl, const std::vector<Gate*>& gates, const NetlistGraph& nlg, const std::vector<std::vector<u32>>& node_features);

    }    // namespace machine_learning
}    // namespace hal