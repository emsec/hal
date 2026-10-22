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

    /**
     * Provides graph representations of a netlist together with the graph algorithms that operate on them.
     */
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
             * `igraph_t` is a plain C struct holding heap pointers, so the implicitly generated copy
             * operations would bitwise-copy `m_graph` and leave two `NetlistGraph` objects aliasing the
             * same graph internals -- the destructor would then call `igraph_destroy()` on them twice, and
             * the copy's `m_graph_ptr` would still point into the source object. Every factory hands out a
             * `std::unique_ptr<NetlistGraph>`, so nothing is meant to copy a graph in the first place;
             * deleting the copy operations makes that invariant explicit and enforced at compile time
             * instead of relying on convention.
             */
            NetlistGraph(const NetlistGraph&) = delete;

            NetlistGraph& operator=(const NetlistGraph&) = delete;

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
             * @brief Create a directed graph from a subset of the gates of a netlist.
             *
             * Only the given gates become vertices and only nets between two of them become edges, so the graph is a
             * closed world irrespective of what the gates are connected to elsewhere in the netlist.
             *
             * Gates in `split_gates` are represented by two vertices instead of one: a primary vertex carrying only the
             * outgoing edges of the gate and a shadow vertex carrying only its incoming edges. This breaks feedback
             * through those gates, which makes a sequential loop such as the round function of a cipher acyclic without
             * having to copy it into a netlist of its own. Both vertices resolve back to the same gate, so use
             * `is_shadow_vertex` to tell the two roles apart. A shadow vertex is only created if the gate actually has
             * incoming edges within the graph.
             *
             * Vertices are numbered in the order of `gates`, so pass them in a deterministic order to obtain a
             * reproducible graph.
             *
             * @param[in] gates - The gates to include in the graph. Must all belong to the same netlist.
             * @param[in] split_gates - The gates to represent by a primary and a shadow vertex. Gates that are not part of `gates` are ignored. Defaults to an empty set.
             * @param[in] filter - An optional filter that is evaluated on every net considered as an edge. Defaults to `nullptr`.
             * @returns The netlist graph on success, an error otherwise.
             */
            static Result<std::unique_ptr<NetlistGraph>>
                from_gates(const std::vector<Gate*>& gates, const std::set<Gate*>& split_gates = {}, const std::function<bool(const Net*)>& filter = nullptr);

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
             * @brief Check whether the specified vertex is a shadow vertex.
             *
             * A shadow vertex carries only the incoming edges of a gate that was split by `from_gates`, while the
             * primary vertex of that gate carries only its outgoing edges. Both resolve to the same gate, so this is
             * the only way to tell which of the two roles a vertex stands for.
             *
             * @param[in] vertex - A vertex.
             * @returns `true` if the vertex is a shadow vertex, `false` otherwise.
             */
            bool is_shadow_vertex(const u32 vertex) const;

            /**
             * @brief Get all vertices corresponding to the specified gate, i.e., its primary vertex and, if the gate
             * was split by `from_gates`, its shadow vertex.
             *
             * @param[in] g - A gate.
             * @returns The vertices of the gate on success, an error otherwise.
             */
            Result<std::vector<u32>> get_all_vertices_from_gate(Gate* g) const;

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
             * Whether `m_graph` has actually been initialized and therefore has to be destroyed.
             *
             * The `NetlistGraph(Netlist*)` constructor deliberately leaves `m_graph` uninitialized -- the
             * factories fill it in afterwards via `igraph_create()`/`igraph_empty()`. If such a factory bails
             * out in between (e.g. an igraph allocation fails), the `std::unique_ptr` holding the
             * half-constructed graph unwinds and the destructor would otherwise call `igraph_destroy()` on
             * uninitialized memory. This flag lets the destructor skip that.
             */
            bool m_graph_initialized = false;

            /**
             * A pointer to the `igraph` object.
             */
            igraph_t* m_graph_ptr = nullptr;

            /**
             * A map from `igraph` nodes to HAL gates.
             */
            std::unordered_map<u32, Gate*> m_nodes_to_gates;

            /**
             * A map from HAL gates to `igraph` nodes.
             */
            std::unordered_map<Gate*, u32> m_gates_to_nodes;

            /**
             * The shadow nodes created by `from_gates`, mapped to the primary node of the same gate.
             *
             * A gate is present in `m_nodes_to_gates` once per node it owns, but only once in `m_gates_to_nodes`,
             * which always holds its primary node. Shadow nodes are therefore tracked separately.
             */
            std::unordered_map<u32, u32> m_shadow_nodes_to_nodes;
        };
    }    // namespace graph_algorithm

    template<>
    std::map<graph_algorithm::NetlistGraph::Direction, std::string> EnumStrings<graph_algorithm::NetlistGraph::Direction>::data;
}    // namespace hal