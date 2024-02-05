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

namespace hal
{
    class NETLIST_API NetlistTraversalDecorator
    {
    public:
        /**
         * Construct new NetlistTraversalDecorator object.
         * 
         * @param[in] netlist - The netlist to operate on.
         */
        NetlistTraversalDecorator(const Netlist& netlist);

        /**
         * TODO deprecate
         * Starting from the given net, get the successor/predecessor gates for which the filter evaluates to `true`.
         * Does not continue traversal beyond gates fulfilling the filter condition, i.e., only the first layer of successors/predecessors is returned.
         * 
         * @param[in] cache - Gate cache to speed up traversal for parts of the netlist that have been traversed before.
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] filter - Filter condition that must be met for the target gates.
         * @param[in] forbidden_pins - Types of pins through which propagation shall not continue. Defaults to en empty set.
         * @returns The next gates fulfilling the filter condition.
         */
        Result<std::unordered_set<Gate*>> get_next_gates(std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache,
                                                         const Net* net,
                                                         bool successors,
                                                         const std::function<bool(const Gate*)>& filter,
                                                         const std::set<PinType>& forbidden_pins = {}) const;

        /**
         * TODO deprecate
         * Starting from the given gate, get the successor/predecessor gates for which the filter evaluates to `true`.
         * Does not continue traversal beyond gates fulfilling the filter condition, i.e., only the first layer of successors/predecessors is returned.
         * 
         * @param[in] cache - Gate cache to speed up traversal for parts of the netlist that have been traversed before.
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] filter - Filter condition that must be met for the target gates.
         * @param[in] forbidden_pins - Types of pins through which propagation shall not continue. Defaults to en empty set.
         * @returns The next gates fulfilling the filter condition.
         */
        Result<std::unordered_set<Gate*>> get_next_gates(std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache,
                                                         const Gate* gate,
                                                         bool successors,
                                                         const std::function<bool(const Gate*)>& filter,
                                                         const std::set<PinType>& forbidden_pins = {}) const;

        /**
         * Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Stop traversal if (1) the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition.
         */
        Result<std::set<Gate*>> get_next_gates_fancy(const Net* net,
                                                     bool successors,
                                                     const std::function<bool(const Gate*)>& target_gate_filter,
                                                     const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                     const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Stop traversal if (1) the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition.
         */
        Result<std::set<Gate*>> get_next_gates_fancy(const Gate* gate,
                                                     bool successors,
                                                     const std::function<bool(const Gate*)>& target_gate_filter,
                                                     const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                     const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        // TODO implement: get_subgraph_gates()

        /**
         * TODO document
         */
        Result<std::set<Net*>> get_subgraph_input_nets(const Net* net,
                                                       bool successors,
                                                       const std::function<bool(const Net*)>& target_net_filter,
                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * TODO document
         */
        Result<std::set<Net*>> get_subgraph_input_nets(const Gate* gate,
                                                       bool successors,
                                                       const std::function<bool(const Net*)>& target_net_filter,
                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

    private:
        const Netlist& m_netlist;
    };
}    // namespace hal