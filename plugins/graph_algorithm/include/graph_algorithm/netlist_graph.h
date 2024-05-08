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

namespace hal
{
    class Netlist;
    class Gate;
    class Net;

    namespace graph_algorithm
    {
        /**
         * Holds a directed graph corresponding to a netlist.
         */
        class NetlistGraph
        {
        public:
            enum class Direction
            {
                NONE,
                IN,
                OUT,
                ALL
            };

                        ~NetlistGraph();

            /**
             * Create a directed graph from a netlist. Optionally create dummy vertices at nets missing a source or destination. An optional filter can be applied to exclude undesired edges.
             * 
             * @param[in] nl - The netlist.
             * @param[in] create_dummy_vertices - Set `true` to create dummy vertices, `false` otherwise. Defaults to `false`.
             * @param[in] filter - An optional filter that is evaluated on every net of the netlist. Defaults to `nullptr`.
             * @returns The netlist graph on success, an error otherwise.
             */
            static Result<std::unique_ptr<NetlistGraph>> from_netlist(Netlist* nl, bool create_dummy_vertices = false, const std::function<bool(const Net*)>& filter = nullptr);

            /**
             * Create an empty directed graph from a netlist, i.e., vertices for all gates are created, but no edges are added.
             * 
             * @param[in] nl - The netlist.
             * @returns The netlist graph on success, an error otherwise.
             */
            static Result<std::unique_ptr<NetlistGraph>> from_netlist_no_edges(Netlist* nl);

            /**
             * Get the netlist associated with the netlist graph.
             * 
             * @returns The netlist.
             */
            Netlist* get_netlist() const;

            /**
             * Get the graph object of the netlist graph.
             * 
             * @returns The graph object.
             */
            igraph_t* get_graph();

            /**
             * Get the gates corresponding to the specified vertices. C must be an iterable container type of `u32` elements.
             * 
             * @tparam C - An iterable container type of `u32` elements.
             * @param[in] vertices - A container of vertices.
             * @returns A vector of gates on success, an error otherwise.
             */
            template<typename C>
            Result<std::vector<Gate*>> get_gates_from_vertices(const C& vertices) const
            {
                std::vector<Gate*> res;
                for (const auto& vertex : vertices)
                {
                    if (const auto it = m_nodes_to_gates.find(vertex); it != m_nodes_to_gates.end())
                    {
                        Gate* g = it->second;
                        if (g != nullptr)
                        {
                            res.push_back(g);
                        }
                        else
                        {
                            log_warning("graph_algorithm", "no gate exists for dummy node {}", vertex);
                        }
                    }
                    else
                    {
                        return ERR("no gate for node " + std::to_string(vertex) + " exists in netlist with ID " + std::to_string(m_nl->get_id()));
                    }
                }
                return OK(res);
            }

            /**
             * Get the gate corresponding to the specified vertex.
             * 
             * @param[in] vertex - A vertex.
             * @returns A gates on success, an error otherwise.
             */
            Result<Gate*> get_gate_from_vertex(const u32 vertex) const;

            /**
             * Get the vertices corresponding to the specified gates.
             * 
             * @tparam C - An iterable container type of `Gate*` elements.
             * @param[in] gates - A container of gates.
             * @returns A vector of vertices on success, an error otherwise.
             */
            template<typename C>
            Result<std::vector<u32>> get_vertices_from_gates(const C& gates) const
            {
                std::vector<u32> res;
                for (const auto& g : gates)
                {
                    if (const auto it = m_gates_to_nodes.find(g); it != m_gates_to_nodes.end())
                    {
                        res.push_back(it->second);
                    }
                    else
                    {
                        return ERR("no node for gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " exists in graph for netlist with ID " + std::to_string(m_nl->get_id()));
                    }
                }
                return OK(res);
            }

            /**
             * Get the vertex corresponding to the specified gate.
             * 
             * @param[in] g - A gate.
             * @returns A vertex on success, an error otherwise.
             */
            Result<u32> get_vertex_from_gate(Gate* g) const;

            /**
             * Get the edges between vertices in the netlist graph.
             * 
             * @returns A vector of edges on success, an error otherwise.
             */
            Result<std::vector<std::pair<u32, u32>>> get_edges() const;

            /**
             * Get the edges between gates in the netlist corresponding to the netlist graph.
             * 
             * @returns A vector of edges on success, an error otherwise.
             */
            Result<std::vector<std::pair<Gate*, Gate*>>> get_edges_in_netlist() const;

            /**
             * Add edges between the specified pairs of source and destination gates to the netlist graph.
             * The gates must already correspond to vertices in the graph.
             * 
             * @param[in] edges - The edges to add as pairs of gates.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> add_edges(const std::vector<std::pair<Gate*, Gate*>>& edges);

            /**
             * Add edges between the specified pairs of source and destination vertices to the netlist graph.
             * The vertices must already exist in the graph.
             * 
             * @param[in] edges - The edges to add as pairs of vertices.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> add_edges(const std::vector<std::pair<u32, u32>>& edges);

            /**
             * Delete edges between the specified pairs of source and destination gates from the netlist graph.
             * 
             * @param[in] edges - The edges to delete as pairs of gates.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> delete_edges(const std::vector<std::pair<Gate*, Gate*>>& edges);

            /**
             * Delete edges between the specified pairs of source and destination vertices from the netlist graph.
             * 
             * @param[in] edges - The edges to delete as pairs of vertices.
             * @returns OK on success, an error otherwise.
             */
            Result<std::monostate> delete_edges(const std::vector<std::pair<u32, u32>>& edges);

            /**
             * Print the edge list of the graph to stdout.
             */
            void print() const;

        private:
            NetlistGraph() = delete;
            NetlistGraph(Netlist* nl);

            Netlist* m_nl;
            igraph_t m_graph;
            std::unordered_map<u32, Gate*> m_nodes_to_gates;
            std::unordered_map<Gate*, u32> m_gates_to_nodes;
        };
    }    // namespace graph_algorithm

    template<>
    std::map<graph_algorithm::NetlistGraph::Direction, std::string> EnumStrings<graph_algorithm::NetlistGraph::Direction>::data;
}    // namespace hal