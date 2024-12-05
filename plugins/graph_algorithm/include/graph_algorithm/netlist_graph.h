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

#include "hal_core/defines.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/result.h"

#include <functional>
#include <igraph/igraph.h>
#include <set>
#include <unordered_map>

/**
 * @file netlist_graph.h 
 * @brief This file contains the class that holds a netlist graph.
 */

namespace hal
{
    class Netlist;
    class Gate;
    class Net;
    class NetlistAbstraction;

    namespace graph_algorithm
    {
        /**
         * @class NetlistGraph
         * @brief A directed graph corresponding to a netlist.
         * 
         * This class holds all information on a netlist graph that corresponds to a gate-level netlist and provides functions to access and operate on it.
         */
        class NetlistGraph
        {
        public:
            /**
             * @enum Direction
             * @brief The direction of exploration within the graph.
             */
            enum class Direction
            {
                /**
                 * @brief No direction, invalid default setting.
                 */
                NONE,

                /**
                 * @brief Explore through the inputs of the current node, i.e., traverse backwards.
                 */
                IN,

                /**
                 * @brief Explore through the outputs of the current node, i.e., traverse forwards.
                 */
                OUT,

                /**
                 * @brief Explore in both directions, i.e., treat the graph as undirected.
                 */
                ALL
            };

            /**
             * @brief Construct a netlist graph from a netlist, an `igraph` graph object, and a map from graph nodes to HAL gates.
             * 
             * @param[in] nl - The netlist.
             * @param[in] graph - The igrapg graph object.
             * @param[in] nodes_to_gates - A map from nodes to gates.
             */
            NetlistGraph(Netlist* nl, igraph_t&& graph, std::unordered_map<u32, Gate*>&& nodes_to_gates);

            /** 
             * @brief Default destructor for `NetlistGraph`.
             */
            ~NetlistGraph();

            /**
             * @brief Create a directed graph from a netlist. 
             * 
             * Optionally create dummy vertices at nets missing a source or destination.
             * An optional filter can be applied to exclude undesired edges.
             * 
             * @param[in] nl - The netlist.
             * @param[in] create_dummy_vertices - Set `true` to create dummy vertices, `false` otherwise. Defaults to `false`.
             * @param[in] filter - An optional filter that is evaluated on every net of the netlist. Defaults to `nullptr`.
             * @returns The netlist graph on success, an error otherwise.
             */
            static Result<std::unique_ptr<NetlistGraph>> from_netlist(Netlist* nl, bool create_dummy_vertices = false, const std::function<bool(const Net*)>& filter = nullptr);

            /**
             * @brief Create an empty directed graph from a netlist.
             * 
             * Vertices for all gates are created, but no edges are added.
             * 
             * @param[in] nl - The netlist.
             * @param[in] gates - The gates to include in the graph. If omitted, all gates of the netlist will be included.
             * @returns The netlist graph on success, an error otherwise.
             */
            static Result<std::unique_ptr<NetlistGraph>> from_netlist_no_edges(Netlist* nl, const std::vector<Gate*>& gates = {});

            /**
             * @brief Create a directed graph from a netlist abstraction. 
             * 
             * Optionally create dummy vertices at global input and output nets
             * 
             * @param[in] nl_abstr - The netlist abstraction.
             * @param[in] create_dummy_vertices - Set `true` to create dummy vertices, `false` otherwise. Defaults to `false`.
             * @returns The netlist graph on success, an error otherwise.
             */
            static Result<std::unique_ptr<NetlistGraph>> from_netlist_abstraction(const NetlistAbstraction* nl_asbtr, const bool create_dummy_vertices = false);

            /**
             * @brief Create a deep copy of the netlist graph.
             * 
             * @returns The copied netlist graph on success, an error otherwise.
             */
            Result<std::unique_ptr<NetlistGraph>> copy() const;

            /**
             * @brief Get the netlist associated with the netlist graph.
             * 
             * @returns The netlist.
             */
            Netlist* get_netlist() const;

            /**
             * @brief Get the graph object of the netlist graph.
             * 
             * @returns The graph object.
             */
            igraph_t* get_graph() const;

            /**
             * @brief Get the gates corresponding to the specified vertices.
             * 
             * The result may contain `nullptr` for dummy vertices.
             * 
             * @param[in] vertices - A vector of vertices.
             * @returns A vector of gates on success, an error otherwise.
             */
            Result<std::vector<Gate*>> get_gates_from_vertices(const std::vector<u32>& vertices) const;

            /**
             * @brief Get the gates corresponding to the specified vertices.
             * 
             * The result may contain `nullptr` for dummy vertices.
             * 
             * @param[in] vertices - A set of vertices.
             * @returns A vector of gates on success, an error otherwise.
             */
            Result<std::vector<Gate*>> get_gates_from_vertices(const std::set<u32>& vertices) const;

            /**
             * @brief Get the gates corresponding to the specified vertices.
             * 
             * The result may contain `nullptr` for dummy vertices.
             * 
             * @param[in] vertices - An igraph vector of vertices.
             * @returns A vector of gates on success, an error otherwise.
             */
            Result<std::vector<Gate*>> get_gates_from_vertices_igraph(const igraph_vector_int_t* vertices) const;

            /**
             * @brief Get the gates corresponding to the specified vertices.
             * 
             * @param[in] vertices - A vector of vertices.
             * @returns A set of gates on success, an error otherwise.
             */
            Result<std::set<Gate*>> get_gates_set_from_vertices(const std::vector<u32>& vertices) const;

            /**
             * @brief Get the gates corresponding to the specified vertices.
             * 
             * @param[in] vertices - A set of vertices.
             * @returns A set of gates on success, an error otherwise.
             */
            Result<std::set<Gate*>> get_gates_set_from_vertices(const std::set<u32>& vertices) const;

            /**
             * @brief Get the gates corresponding to the specified vertices.
             * 
             * @param[in] vertices - An igraph vector of vertices.
             * @returns A set of gates on success, an error otherwise.
             */
            Result<std::set<Gate*>> get_gates_set_from_vertices_igraph(const igraph_vector_int_t* vertices) const;

            /**
             * @brief Get the gate corresponding to the specified vertex.
             * 
             * @param[in] vertex - A vertex.
             * @returns A gates on success, an error otherwise.
             */
            Result<Gate*> get_gate_from_vertex(const u32 vertex) const;

            /**
             * @brief Get the vertices corresponding to the specified gates.
             * 
             * @param[in] gates - A vector of gates.
             * @returns A vector of vertices on success, an error otherwise.
             */
            Result<std::vector<u32>> get_vertices_from_gates(const std::vector<Gate*>& gates) const;

            /**
             * @brief Get the vertices corresponding to the specified gates.
             * 
             * @param[in] gates - A set of gates.
             * @returns A vector of vertices on success, an error otherwise.
             */
            Result<std::vector<u32>> get_vertices_from_gates(const std::set<Gate*>& gates) const;

            /**
             * @brief Get the vertices corresponding to the specified gates.
             * 
             * @param[in] gates - A vector of gates.
             * @returns An igraph vector of vertices on success, an error otherwise.
             */
            Result<igraph_vector_int_t> get_vertices_from_gates_igraph(const std::vector<Gate*>& gates) const;

            /**
             * @brief Get the vertices corresponding to the specified gates.
             * 
             * @param[in] gates - A set of gates.
             * @returns An igraph vector of vertices on success, an error otherwise.
             */
            Result<igraph_vector_int_t> get_vertices_from_gates_igraph(const std::set<Gate*>& gates) const;

            /**
             * @brief Get the vertex corresponding to the specified gate.
             * 
             * @param[in] g - A gate.
             * @returns A vertex on success, an error otherwise.
             */
            Result<u32> get_vertex_from_gate(Gate* g) const;

            /**
             * @brief Get the number of vertices in the netlist graph.
             * 
             * @param[in] only_connected - Set `true` to only count vertices connected to at least one edge, `false` otherwise. Defaults to `false`.
             * @returns The number of vertices in the netlist graph.
             */
            u32 get_num_vertices(bool only_connected = false) const;

            /**
             * @brief Get the number of edges in the netlist graph.
             * 
             * @returns The number of edges in the netlist graph.
             */
            u32 get_num_edges() const;

            /**
             * @brief Get the vertices in the netlist graph.
             * 
             * @param[in] only_connected - Set `true` to only return vertices connected to at least one edge, `false` otherwise. Defaults to `false`.
             * @returns A vector of vertices on success, an error otherwise.
             */
            Result<std::vector<u32>> get_vertices(bool only_connected = false) const;

            /**
             * @brief Get the edges between vertices in the netlist graph.
             * 
             * @returns A vector of edges on success, an error otherwise.
             */
            Result<std::vector<std::pair<u32, u32>>> get_edges() const;

            /**
             * @brief Get the edges between gates in the netlist corresponding to the netlist graph.
             * 
             * @returns A vector of edges on success, an error otherwise.
             */
            Result<std::vector<std::pair<Gate*, Gate*>>> get_edges_in_netlist() const;

            /**
             * @brief Add edges between the specified pairs of source and destination gates to the netlist graph.
             * 
             * The gates must already correspond to vertices in the graph.
             * 
             * @param[in] edges - The edges to add as pairs of gates.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> add_edges(const std::vector<std::pair<Gate*, Gate*>>& edges);

            /**
             * @brief Add edges between the specified pairs of source and destination vertices to the netlist graph.
             * 
             * The vertices must already exist in the graph.
             * 
             * @param[in] edges - The edges to add as pairs of vertices.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> add_edges(const std::vector<std::pair<u32, u32>>& edges);

            /**
             * @brief Add edges between the specified pairs of source and destination gates to the netlist graph.
             * 
             * The vertices must already exist in the graph.
             * 
             * @param[in] edges - The edges to add as a map from source gate to its destination gates.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> add_edges(const std::map<Gate*, std::set<Gate*>>& edges);

            /**
             * @brief Delete edges between the specified pairs of source and destination gates from the netlist graph.
             * 
             * @param[in] edges - The edges to delete as pairs of gates.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> delete_edges(const std::vector<std::pair<Gate*, Gate*>>& edges);

            /**
             * @brief Delete edges between the specified pairs of source and destination vertices from the netlist graph.
             * 
             * @param[in] edges - The edges to delete as pairs of vertices.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> delete_edges(const std::vector<std::pair<u32, u32>>& edges);

            /**
             * @brief Print the edge list of the graph to stdout.
             */
            void print() const;

        private:
            NetlistGraph() = delete;

            /**
             * @brief Construct an empty netlist graph from a netlist.
             * 
             * @param[in] nl - The netlist.
             */
            NetlistGraph(Netlist* nl);

            /**
             * The netlist to which the graph corresponds. 
             */
            Netlist* m_nl;

            /**
             * The `igraph` object corresponding to the netlist.
             */
            igraph_t m_graph;

            /**
             * A pointer to the `igraph` object.
             */
            igraph_t* m_graph_ptr;

            /**
             * A map from `igraph` nodes to HAL gates.
             */
            std::unordered_map<u32, Gate*> m_nodes_to_gates;

            /**
             * A map from HAL gates to `igraph` nodes. 
             */
            std::unordered_map<Gate*, u32> m_gates_to_nodes;
        };
    }    // namespace graph_algorithm

    template<>
    std::map<graph_algorithm::NetlistGraph::Direction, std::string> EnumStrings<graph_algorithm::NetlistGraph::Direction>::data;
}    // namespace hal