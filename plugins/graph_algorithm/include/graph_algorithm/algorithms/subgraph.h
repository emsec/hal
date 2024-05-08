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
#include "hal_core/utilities/result.h"

#include <igraph/igraph.h>
#include <set>

namespace hal
{
    class Gate;

    namespace graph_algorithm
    {
        class NetlistGraph;

        /**
         * Compute the subgraph induced by the specified gates, including all edges between the corresponding vertices.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] subgraph_gates - A vector of gates that make up the subgraph.
         * @returns The subgraph as a new netlist graph on success, an error otherwise.
         */
        Result<std::unique_ptr<NetlistGraph>> get_subgraph(NetlistGraph* graph, const std::vector<Gate*>& subgraph_gates);

        /**
         * Compute the subgraph induced by the specified vertices, including all edges between these vertices.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] subgraph_vertices - A vector of vertices that make up the subgraph.
         * @returns The subgraph as a new netlist graph on success, an error otherwise.
         */
        Result<std::unique_ptr<NetlistGraph>> get_subgraph(NetlistGraph* graph, const std::vector<u32>& subgraph_vertices);

        /**
         * Compute the subgraph induced by the specified vertices, including all edges between these vertices.
         * 
         * @param[in] graph - The netlist graph.
         * @param[in] subgraph_vertices - An igraph vector of vertices that make up the subgraph.
         * @returns The subgraph as a new netlist graph on success, an error otherwise.
         */
        Result<std::unique_ptr<NetlistGraph>> get_subgraph_igraph(NetlistGraph* graph, const igraph_vector_int_t* subgraph_vertices);
    }    // namespace graph_algorithm
}    // namespace hal