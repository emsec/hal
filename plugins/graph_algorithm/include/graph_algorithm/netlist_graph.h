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
        class NetlistGraph
        {
        public:
            ~NetlistGraph();

            static Result<std::unique_ptr<NetlistGraph>> from_netlist(Netlist* nl, bool create_dummy_nodes = false, const std::function<bool(const Net*)>& filter = nullptr);

            // TODO implement
            static Result<std::unique_ptr<NetlistGraph>> from_netlist_no_edges(Netlist* nl);

            Netlist* get_netlist() const;
            igraph_t* get_graph() const;

            Result<Gate*> get_gate_of_vertex(const u32 node) const;
            Result<std::vector<Gate*>> get_gates_of_vertices(const std::set<u32>& nodes) const;
            Result<std::vector<Gate*>> get_gates_of_vertices(const std::vector<u32>& nodes) const;

            Result<u32> get_vertex_of_gate(Gate* g) const;

            // TODO implement 4 below
            void add_edges(const std::vector<std::pair<Gate*, Gate*>>& edges);
            void add_edges(const std::vector<std::pair<u32, u32>>& edges);
            void add_edges(const std::vector<Net*>& edges);
            void delete_edges(const std::vector<std::pair<Gate*, Gate*>>& edges);
            void delete_edges(const std::vector<std::pair<u32, u32>>& edges);
            void delete_edges(const std::vector<Net*>& edges);

        private:
            NetlistGraph() = delete;
            NetlistGraph(Netlist* nl);

            Netlist* m_nl;
            igraph_t* m_graph;
            std::unordered_map<u32, Gate*> m_nodes_to_gates;
            std::unordered_map<Gate*, u32> m_gates_to_nodes;
        };
    }    // namespace graph_algorithm
}    // namespace hal