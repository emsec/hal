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
    /**
     * A netlist decorator that provides functionality to traverse the associated netlist without making any modifications.
     *
     * @ingroup decorators
     */
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
         * Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Traverse over gates that do not meet the `target_gate_filter` condition.
         * Stop traversal if (1) `continue_on_match` is `false` the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist using the same forbidden pins.
         * Do not use a cache if the filter functions operate on the `current_depth`.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] continue_on_match - Set `true` to continue even if `target_gate_filter` evaluated to `true`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates(const Net* net,
                                                        bool successors,
                                                        const std::function<bool(const Gate*)>& target_gate_filter,
                                                        bool continue_on_match                                                                     = false,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr,
                                                        std::unordered_map<const Net*, std::set<Gate*>>* cache                                     = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Traverse over gates that do not meet the `target_gate_filter` condition.
         * Stop traversal if (1) `continue_on_match` is `false` the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * Do not use a cache if the filter functions operate on the `current_depth`.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] continue_on_match - Set `true` to continue even if `target_gate_filter` evaluated to `true`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates(const Gate* gate,
                                                        bool successors,
                                                        const std::function<bool(const Gate*)>& target_gate_filter,
                                                        bool continue_on_match                                                                     = false,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                        const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr,
                                                        std::unordered_map<const Net*, std::set<Gate*>>* cache                                     = nullptr) const;

        /**
         * Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Continue traversal independent of whatever `target_gate_filter` evaluates to.
         * Stop traversal if (1) `continue_on_mismatch` is `false` the `target_gate_filter` evaluates to `false`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] continue_on_mismatch - Set `true` to continue even if `target_gate_filter` evaluated to `false`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until(const Net* net,
                                                              bool successors,
                                                              const std::function<bool(const Gate*)>& target_gate_filter,
                                                              bool continue_on_mismatch                                                                  = false,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Continue traversal independent of whatever `target_gate_filter` evaluates to.
         * Stop traversal if (1) `continue_on_mismatch` is `false` the `target_gate_filter` evaluates to `false`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] continue_on_mismatch - Set `true` to continue even if `target_gate_filter` evaluated to `false`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until(const Gate* gate,
                                                              bool successors,
                                                              const std::function<bool(const Gate*)>& target_gate_filter,
                                                              bool continue_on_mismatch                                                                  = false,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Continue traversal independent of whatever `target_gate_filter` evaluates to.
         * Stop traversal if the specified depth is reached.
         * The current depth is counted starting at 1 for the destinations of the provided net. 
         * For a `depth` of `0`, all nets between the start gate and the global netlist outputs will be traversed.
         * The target_gate_filter may be omitted in which case all traversed gates will be returned.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] max_depth - The maximum depth for netlist traversal starting from the start net.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until_depth(const Net* net, bool successors, u32 max_depth, const std::function<bool(const Gate*)>& target_gate_filter = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Continue traversal independent of whatever `target_gate_filter` evaluates to.
         * Stop traversal if the specified depth is reached.
         * The current depth is counted starting at 1 for the direct successors/predecessors of the provided gate. 
         * For a `depth` of `0`, all gates between the start gate and the global netlist outputs will be traversed.
         * The target_gate_filter may be omitted in which case all traversed gates will be returned.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] max_depth - The maximum depth for netlist traversal starting from the start gate.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates_until_depth(const Gate* gate, bool successors, u32 max_depth, const std::function<bool(const Gate*)>& target_gate_filter = nullptr) const;

        /**
         * Starting from the given net, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
         * Traverse over gates that are not sequential until a sequential gate is found.
         * Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist using the same forbidden pins.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Sequential gates reached through these pins will not be part of the result.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next sequential gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_sequential_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
         * Traverse over gates that are not sequential until a sequential gate is found.
         * Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist using the same forbidden pins.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Sequential gates reached through these pins will not be part of the result.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next sequential gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_sequential_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Get the next sequential gates for all sequential gates in the netlist by traversing through remaining logic (e.g., combinational logic).
         * Compute a map from a sequential gate to all its successors.
         * Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
         * 
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Sequential gates reached through these pins will not be part of the result.
         * @returns A map from each sequential gate to all its sequential successors on success, an error otherwise.
         */
        Result<std::map<Gate*, std::set<Gate*>>> get_next_sequential_gates_map(bool successors, const std::set<PinType>& forbidden_pins) const;

        /**
         * Starting from the given net, traverse the netlist and return all combinational successor/predecessor gates.
         * Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
         * All combinational gates found during traversal are added to the result.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next combinational gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_combinational_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return all combinational successor/predecessor gates.
         * Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
         * All combinational gates found during traversal are added to the result.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next combinational gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_combinational_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        // TODO move get_path and get_shortest_path here

        // TODO move get_gate_chain and get_complex_gate_chain here

    private:
        const Netlist& m_netlist;
    };
}    // namespace hal