// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
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
#include <vector>

namespace hal
{
    class Netlist;
    class Gate;
    class Net;
    class Endpoint;

    namespace cte
    {
        enum PtrType { UNKNOWN, GATE, NET };

        enum class Severity { debug, info, warning, error };

        enum class DiagnosticCode {
            invalid_clock_pin_count,
            unconnected_clock_pin,
            invalid_clock_net_sources,
            unrouted_net,
            multi_driven_net,
            enqueued_net,
            multi_input_gate,
            combinational_clock_sink,
            clock_on_non_clock_pin
        };

        struct Diagnostic
        {
            Severity severity;
            DiagnosticCode code;
            PtrType subject_type;
            const void *subject_ptr;
            u32 subject_id;
            std::string message;
            // for diagnostics caused by a single connection: the endpoint through which the net reaches the
            // subject gate, which also gives the pin. nullptr for all other diagnostics
            const Endpoint *endpoint = nullptr;
        };

        // a root-to-sink path through the clock tree
        using Branch = std::vector<std::pair<const void *, PtrType>>;

        struct VoidPtrHash
        {
            std::size_t operator()( const std::pair<const void *, const void *> &pair ) const noexcept
            {
                return std::hash<const void *>()( pair.first ) ^ ( std::hash<const void *>()( pair.second ) << 1 );
            }
        };

        struct PairPtrEq
        {
            bool operator()( const std::pair<const void *, const void *> &p1,
                             const std::pair<const void *, const void *> &p2 ) const noexcept
            {
                return p1.first == p2.first && p1.second == p2.second;
            }
        };

        class ClockTree
        {
          public:
            ~ClockTree();

            static Result<std::unique_ptr<ClockTree>> from_netlist( const Netlist *netlist );

            Result<std::vector<std::pair<const void *, PtrType>>> get_neighbors( const void *ptr,
                                                                                 igraph_neimode_t direction ) const;

            Result<std::unique_ptr<ClockTree>> get_subtree( const void *ptr, bool parent ) const;

            Result<igraph_int_t> get_vertex_from_ptr( const void *ptr ) const;

            [[nodiscard]] Result<std::pair<const void *, PtrType>> get_ptr_from_vertex( igraph_int_t vertex ) const;

            [[nodiscard]] Result<std::vector<igraph_int_t>>
            get_vertices_from_ptrs( const std::vector<const void *> &ptrs ) const;

            [[nodiscard]] Result<std::vector<std::pair<const void *, PtrType>>>
            get_ptrs_from_vertices( const std::vector<igraph_int_t> &vertices ) const;

            [[nodiscard]] std::vector<const Gate *> get_gates() const;

            [[nodiscard]] std::vector<const Net *> get_nets() const;

            [[nodiscard]] const std::unordered_map<const void *, PtrType> &get_all() const;

            [[nodiscard]] const Netlist *get_netlist() const;

            [[nodiscard]] const igraph_t *get_igraph() const;

            [[nodiscard]] const std::vector<Diagnostic> &get_diagnostics() const;

            [[nodiscard]] const std::unordered_set<igraph_int_t> &get_roots() const;

          private:
            // takes ownership of graph, which must be initialized; only from_netlist and get_subtree create trees
            ClockTree( const Netlist *netlist,
                       igraph_t &&graph,
                       std::unordered_set<igraph_int_t> &&roots,
                       std::unordered_map<igraph_int_t, const void *> &&vertices_to_ptrs,
                       std::unordered_map<const void *, PtrType> &&ptrs_to_types );

            const Netlist *m_netlist;

            igraph_t m_igraph;

            igraph_t *m_igraph_ptr;

            std::unordered_set<igraph_int_t> m_roots;

            std::unordered_map<igraph_int_t, const void *> m_vertices_to_ptrs;

            std::unordered_map<const void *, igraph_int_t> m_ptrs_to_vertices;

            std::unordered_map<const void *, PtrType> m_ptrs_to_types;

            std::vector<Diagnostic> m_diagnostics;
        };
    }  // namespace cte
}  // namespace hal
