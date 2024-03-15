// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

#include <filesystem>
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
         * Generates an directed graph based on the current netlist. Each gate is transformed to a node while each
         * net is transformed to an edge. The function returns the mapping from igraph node ids to HAL gates. Note
         * that for each global input and output dummy nodes are generated in the igraph representation.
         *
         * @param[in] netlist - The netlist to operate on.
         * @param[out] graph - The output igraph object.
         * @returns map from igraph node id to HAL gate ID, to be able to match back any graph operations.
         */
        std::map<int, Gate*> get_igraph_directed(Netlist* const netlist, igraph_t* graph);

        /**
         * Generates an directed graph based on the current netlist with only the dependency of the FFs.
         * Each FF is transformed to a node while each net is transformed to an edge. The function returns
         * the mapping from igraph node ids to HAL gates. 
         * 
         * @param[in] netlist - The netlist to operate on.
         * @param[in] igraph - igraph object
         * @returns map from igraph node id to HAL gate ID, to be able to match back any graph operations.
         */
        std::map<int, Gate*> get_igraph_ff_dependency(Netlist* const nl, igraph_t* graph);

        /**
         * Uses the mapping provided by the the get_igraph_directed() function to generate sets of HAL gates
         * that were generated by the clustering algorithms of igraph. The igraph membership vector contains
         * the generated clusters from the igraph framework, which is used to generate the sets of gates in HAL.
         * The sets are stored in a map with the regarding cluster ID from igraph, since these can contain information
         * generated by the clustering algorithm.
         *
         * @param[in] graph - igraph graph object
         * @param[in] membership - membership vector
         * @param[in] vertex_to_gate - map from node ID in igraph to HAL gate
         * @returns map from membership id to set of gates that have the membership.
         */
        std::map<int, std::set<Gate*>> get_memberships_for_hal(igraph_t* graph, igraph_vector_t membership, std::map<int, Gate*> vertex_to_gate);

        /**
         * Creates a graph edgelist file based on the current igraph
         *
         * @param[in] graph - igraph graph object
         * @param[in] output_file - output file path
         * @returns map from membership id to set of gates that have the membership.
         */
        bool write_graph_to_file(igraph_t* graph, const std::string& output_file);

        /**
         * Creates a graph edgelist file based on the current netlist with only the FF dependencies
         *
         * @param[in] netlist - netlist
         * @param[in] output_file - output file path
         * @returns map from membership id to set of gates that have the membership.
         */
        bool write_ff_dependency_graph(Netlist* nl, const std::string& output_file);
    };

    namespace graph_algorithm
    {
        /**
         * TODO
         *
         * @param[out] igraph - The output igraph object.
         */
        void get_igraph_directed(const std::vector<std::pair<u32, u32>>& edges, igraph_t* graph);

        /**
         * TODO
         *
         * @param[out] igraph - The output igraph object.
         */
        void add_edges(const std::vector<std::pair<u32, u32>>& edges, igraph_t* graph);

        /**
         * Get a vector of strongly connected components (SCC) with each SSC being represented by a vector of gates.
         *
         * @param[in] graph - The igraph object to operate on.
         * @returns A set of SCCs.
         */
        std::set<std::set<u32>> get_strongly_connected_components(igraph_t* graph);
    }    // namespace graph_algorithm
}    // namespace hal