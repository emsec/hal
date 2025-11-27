// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS").
// All Rights reserved.
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
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace hal
{
    class Netlist;
}

namespace hal
{
    class Gate;
}

namespace hal
{
    class Net;
}

namespace hal
{
    namespace cte
    {
        enum PtrType { UNKNOWN, GATE, NET };

        class ClockTree
        {
          public:
            ClockTree( const Netlist *netlist,
                       igraph_t &&graph,
                       std::unordered_set<igraph_integer_t> &&roots,
                       std::unordered_map<igraph_integer_t, const void *> &&m_vertices_to_ptrs,
                       std::unordered_map<const void *, PtrType> &&m_ptrs_to_types );

            ~ClockTree();

            static Result<std::unique_ptr<ClockTree>> from_netlist( const Netlist *netlist );

            Result<std::monostate> export_dot( const std::string &pathname ) const;

            Result<std::unique_ptr<ClockTree>> get_subtree( const void *ptr, const bool parent ) const;

            Result<igraph_integer_t> get_vertex_from_ptr( const void *ptr ) const;

            Result<std::pair<const void *, PtrType>> get_ptr_from_vertex( const igraph_integer_t vertex ) const;

            Result<std::vector<igraph_integer_t>> get_vertices_from_ptrs( const std::vector<const void *> &ptrs ) const;

            Result<std::vector<std::pair<const void *, PtrType>>>
            get_ptrs_from_vertices( const std::vector<igraph_integer_t> &vertices ) const;

            const std::vector<const Gate *> get_gates() const;

            const std::vector<const Net *> get_nets() const;

            const std::unordered_map<const void *, PtrType> get_all() const;

            const Netlist *get_netlist() const;

            const igraph_t *get_igraph() const;

          private:
            ClockTree() = delete;

            ClockTree( const Netlist *netlist );

            const Netlist *m_netlist;

            igraph_t m_igraph;

            igraph_t *m_igraph_ptr;

            std::unordered_set<igraph_integer_t> m_roots;

            std::unordered_map<igraph_integer_t, const void *> m_vertices_to_ptrs;

            std::unordered_map<const void *, igraph_integer_t> m_ptrs_to_vertices;

            std::unordered_map<const void *, PtrType> m_ptrs_to_types;
        };
    }  // namespace cte
}  // namespace hal
