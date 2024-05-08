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

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <igraph/igraph.h>
#include <set>

namespace hal
{
    class Gate;

    namespace graph_algorithm
    {
        /**
         * Compute a shortest path from the specified `from_gate` to each of the given `to_gates` by traversing in the provided direction.
         * Returns one shortest path for each end gate, even if multiple shortest paths exist.
         * Each shortest path is given as a vector of vertices in the order of traversal.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] from_gate - The start gate of the shortest path.
         * @param[in] to_gates - A vector of end gates of the shortest path.
         * @param[in] direction - The direction in which to compute the shortest paths starting at the `from_gate`.
         * @returns The shortest paths in order of the `to_gates` on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_shortest_paths(NetlistGraph* graph, Gate* from_gate, const std::vector<Gate*>& to_gates, NetlistGraph::Direction direction);

        /**
         * Compute a shortest path from the specified `from_vertex` to each of the given `to_vertices` by traversing in the provided direction.
         * Returns one shortest path for each end vertex, even if multiple shortest paths exist.
         * Each shortest path is given as a vector of vertices in the order of traversal.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] from_gate - The start vertex of the shortest path.
         * @param[in] to_gates - A vector of end vertices of the shortest path.
         * @param[in] direction - The direction in which to compute the shortest paths starting at the `from_vertex`.
         * @returns The shortest paths in order of the `to_vertices` on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_shortest_paths(NetlistGraph* graph, u32 from_vertex, const std::vector<u32>& to_vertices, NetlistGraph::Direction direction);

        /**
         * Compute a shortest path from the specified `from_vertex` to each of the given `to_vertices` by traversing in the provided direction.
         * Returns one shortest path for each end vertex, even if multiple shortest paths exist.
         * Each shortest path is given as a vector of vertices in the order of traversal.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] from_gate - The start vertex of the shortest path.
         * @param[in] to_gates - An igraph vector of end vertices of the shortest path.
         * @param[in] direction - The direction in which to compute the shortest paths starting at the `from_vertex`.
         * @returns The shortest paths in order of the `to_vertices` on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_shortest_paths_igraph(NetlistGraph* graph, u32 from_vertex, const igraph_vector_int_t* to_vertices, NetlistGraph::Direction direction);

        /**
         * Compute a shortest path from the specified `from_gate` to each of the given `to_gates` by traversing in the provided direction.
         * Returns all shortest paths for each end gate.
         * Each shortest path is given as a vector of vertices in the order of traversal.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] from_gate - The start gate of the shortest path.
         * @param[in] to_gates - A vector of end gates of the shortest path.
         * @param[in] direction - The direction in which to compute the shortest paths starting at the `from_gate`.
         * @returns The shortest paths in order of the `to_gates` on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_all_shortest_paths(NetlistGraph* graph, Gate* from_gate, const std::vector<Gate*>& to_gates, NetlistGraph::Direction direction);

        /**
         * Compute a shortest path from the specified `from_vertex` to each of the given `to_vertices` by traversing in the provided direction.
         * Returns all shortest paths for each end gate.
         * Each shortest path is given as a vector of vertices in the order of traversal.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] from_gate - The start vertex of the shortest path.
         * @param[in] to_gates - A vector of end vertices of the shortest path.
         * @param[in] direction - The direction in which to compute the shortest paths starting at the `from_vertex`.
         * @returns The shortest paths in order of the `to_vertices` on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_all_shortest_paths(NetlistGraph* graph, u32 from_vertex, const std::vector<u32>& to_vertices, NetlistGraph::Direction direction);

        /**
         * Compute a shortest path from the specified `from_vertex` to each of the given `to_vertices` by traversing in the provided direction.
         * Returns all shortest paths for each end gate.
         * Each shortest path is given as a vector of vertices in the order of traversal.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] from_gate - The start vertex of the shortest path.
         * @param[in] to_gates - An igraph vector of end vertices of the shortest path.
         * @param[in] direction - The direction in which to compute the shortest paths starting at the `from_vertex`.
         * @returns The shortest paths in order of the `to_vertices` on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_all_shortest_paths_igraph(NetlistGraph* graph, u32 from_vertex, const igraph_vector_int_t* to_vertices, NetlistGraph::Direction direction);
    }    // namespace graph_algorithm
}    // namespace hal