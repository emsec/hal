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

/**
 * @file neighborhood.h 
 * @brief This file contains functions related to neighborhoods in graphs.
 */

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
         * @brief Compute the neighborhood of the given order for each of the specified gates within the given netlist graph.
         * 
         * For order 0, only the vertex itself is returned. For order 1, the vertex itself and all vertices that are its direct predecessors and/or successors (depending on the specified direction). For order 2, the neighborhood of order 1 plus all direct predecessors and/or successors of the vertices in order 1 are returned, etc.
         * Returns each neighborhood as a vector of vertices in the netlist graph.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] start_gates - A vector of gates for which to compute the neighborhood.
         * @param[in] order - The order of the neighborhood to compute.
         * @param[in] direction - The direction in which the neighborhood should be computed.
         * @param[in] min_dist - The minimum distance of the vertices to include in the result.
         * @returns A vector of neighborhoods of each of the provided start gates (in order) on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_neighborhood(NetlistGraph* graph, const std::vector<Gate*>& start_gates, u32 order, NetlistGraph::Direction direction, u32 min_dist = 0);

        /**
         * @brief Compute the neighborhood of the given order for each of the specified vertices within the given netlist graph.
         * 
         * For order 0, only the vertex itself is returned. For order 1, the vertex itself and all vertices that are its direct predecessors and/or successors (depending on the specified direction). For order 2, the neighborhood of order 1 plus all direct predecessors and/or successors of the vertices in order 1 are returned, etc.
         * Returns each neighborhood as a vector of vertices in the netlist graph.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] start_vertices - A vector of vertices for which to compute the neighborhood.
         * @param[in] order - The order of the neighborhood to compute.
         * @param[in] direction - The direction in which the neighborhood should be computed.
         * @param[in] min_dist - The minimum distance of the vertices to include in the result.
         * @returns A vector of neighborhoods of each of the provided start vertices (in order) on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_neighborhood(NetlistGraph* graph, const std::vector<u32>& start_vertices, u32 order, NetlistGraph::Direction direction, u32 min_dist = 0);

        /**
         * @brief Compute the neighborhood of the given order for each of the specified vertices within the given netlist graph.
         * 
         * For order 0, only the vertex itself is returned. For order 1, the vertex itself and all vertices that are its direct predecessors and/or successors (depending on the specified direction). For order 2, the neighborhood of order 1 plus all direct predecessors and/or successors of the vertices in order 1 are returned, etc.
         * Returns each neighborhood as a vector of vertices in the netlist graph.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] start_vertices - An igraph vector of vertices for which to compute the neighborhood.
         * @param[in] order - The order of the neighborhood to compute.
         * @param[in] direction - The direction in which the neighborhood should be computed.
         * @param[in] min_dist - The minimum distance of the vertices to include in the result.
         * @returns A vector of neighborhoods of each of the provided start vertices (in order) on success, an error otherwise.
         */
        Result<std::vector<std::vector<u32>>> get_neighborhood_igraph(NetlistGraph* graph, const igraph_vector_int_t* start_vertices, u32 order, NetlistGraph::Direction direction, u32 min_dist = 0);
    }    // namespace graph_algorithm
}    // namespace hal