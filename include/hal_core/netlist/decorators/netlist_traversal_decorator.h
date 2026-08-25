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

#include <optional>

namespace hal
{
    /**
     * The direction in which a netlist is traversed.
     *
     * @ingroup decorators
     */
    enum class TraversalDirection
    {
        forward,  /**< Follow the fan-out, i.e., towards the successors of a gate. */
        backward, /**< Follow the fan-in, i.e., towards the predecessors of a gate. */
        both,     /**< Follow both, and report the union of the two. */
    };

    /**
     * Where a traversal stops relative to the gates it is looking for.
     *
     * A traversal walks the netlist collecting the gates a filter accepts. What separates one
     * traversal from another is not what it collects but where it comes to a halt, and these are the
     * three ways that can be answered.
     *
     * @ingroup decorators
     */
    enum class TraversalStop
    {
        /**
         * Stop at a gate the filter accepts. The gates collected are the boundary of the search: they
         * are reported but not traversed through, so what lies behind them is not reached. This is
         * how one asks for the next flip-flops behind a cone of combinational logic.
         */
        at_match,

        /**
         * Stop at a gate the filter rejects. Only gates the filter accepts are traversed through, so
         * the gates collected form one connected region of them. This is how one asks for the
         * combinational logic between two registers.
         */
        at_mismatch,

        /**
         * Do not stop at a gate at all. Everything reachable is traversed and every gate the filter
         * accepts is collected on the way. Bound this with a depth or with the endpoint filters,
         * or it walks to the edges of the netlist.
         */
        never,
    };

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
         * Traverse the netlist from the given net, collecting the gates that `match` accepts.
         *
         * This is the traversal that the other functions of this decorator are written in terms of. What
         * distinguishes them from one another is `stop`, which says where the walk halts relative to the
         * gates being looked for, see `TraversalStop`.
         *
         * @param[in] net - The net to start from.
         * @param[in] direction - The direction to traverse in.
         * @param[in] match - The condition a gate has to meet to be collected.
         * @param[in] stop - Where to stop traversing, relative to the gates that `match` accepts.
         * @param[in] max_depth - The maximum number of gates to traverse through, counted from 1 for the direct neighbours of the start. `0` for no limit.
         * @param[in] exit_endpoint_filter - Condition that has to hold to leave a gate through a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Condition that has to hold to enter a gate through a successor/predecessor endpoint.
         * @returns The gates that were collected on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_gates(const Net* net,
                                          TraversalDirection direction,
                                          const std::function<bool(const Gate*)>& match,
                                          TraversalStop stop,
                                          u32 max_depth                                                                                = 0,
                                          const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                          const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Traverse the netlist from the given gate, collecting the gates that `match` accepts.
         *
         * This is the traversal that the other functions of this decorator are written in terms of. What
         * distinguishes them from one another is `stop`, which says where the walk halts relative to the
         * gates being looked for, see `TraversalStop`.
         *
         * @param[in] gate - The gate to start from.
         * @param[in] direction - The direction to traverse in.
         * @param[in] match - The condition a gate has to meet to be collected.
         * @param[in] stop - Where to stop traversing, relative to the gates that `match` accepts.
         * @param[in] max_depth - The maximum number of gates to traverse through, counted from 1 for the direct neighbours of the start. `0` for no limit.
         * @param[in] exit_endpoint_filter - Condition that has to hold to leave a gate through a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Condition that has to hold to enter a gate through a successor/predecessor endpoint.
         * @returns The gates that were collected on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_gates(const Gate* gate,
                                          TraversalDirection direction,
                                          const std::function<bool(const Gate*)>& match,
                                          TraversalStop stop,
                                          u32 max_depth                                                                                = 0,
                                          const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                          const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Traverse over gates that do not meet the `target_gate_filter` condition.
         *
         * Equivalent to `get_gates` with a `TraversalStop` of `at_match`, or of `never` when `continue_on_match` is set.
         * Stop traversal if (1) `continue_on_match` is `false` the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] continue_on_match - Set `true` to continue even if `target_gate_filter` evaluated to `true`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates(const Net* net,
                                                        bool successors,
                                                        const std::function<bool(const Gate*)>& target_gate_filter,
                                                        bool continue_on_match                                                                     = false,
                                                        const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                        const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Traverse over gates that do not meet the `target_gate_filter` condition.
         * Stop traversal if (1) `continue_on_match` is `false` the `target_gate_filter` evaluates to `true`, (2) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (3) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * Both the `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] continue_on_match - Set `true` to continue even if `target_gate_filter` evaluated to `true`, `false` otherwise. Defaults to `false`.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition on success, an error otherwise.
         */
        Result<std::set<Gate*>> get_next_matching_gates(const Gate* gate,
                                                        bool successors,
                                                        const std::function<bool(const Gate*)>& target_gate_filter,
                                                        bool continue_on_match                                                                     = false,
                                                        const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                        const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

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
                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

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
                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

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
         * @param[in] forbidden_pins - Sequential gates reached through these pins will not be part of the result. Defaults to an empty set.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next sequential gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_sequential_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins = {}, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the next layer of sequential successor/predecessor gates.
         * Traverse over gates that are not sequential until a sequential gate is found.
         * Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist using the same forbidden pins.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Sequential gates reached through these pins will not be part of the result. Defaults to an empty set.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next sequential gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_sequential_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins = {}, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Get the next sequential gates for all sequential gates in the netlist by traversing through remaining logic (e.g., combinational logic).
         * Compute a map from a sequential gate to all its successors.
         * Stop traversal at all sequential gates, but only adds those to the result that have not been reached through a pin of one of the forbidden types.
         * 
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Sequential gates reached through these pins will not be part of the result. Defaults to an empty set.
         * @returns A map from each sequential gate to all its sequential successors on success, an error otherwise.
         */
        Result<std::map<Gate*, std::set<Gate*>>> get_next_sequential_gates_map(bool successors, const std::set<PinType>& forbidden_pins) const;

        /**
         * Starting from the given net, traverse the netlist and return all combinational successor/predecessor gates.
         * Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
         * All combinational gates found during traversal are added to the result.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist.
         * Forbidden pins can be provided to, e.g., avoid the inclusion of logic in front of flip-flop control inputs.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Traversal stops at pins of these types, i.e., gates reached through such a pin are not part of the result. Defaults to an empty set.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next combinational gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_combinational_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins = {}, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return all combinational successor/predecessor gates.
         * Continue traversal as long as further combinational gates are found and stop at gates that are not combinational.
         * All combinational gates found during traversal are added to the result.
         * Provide a cache to speed up traversal when calling this function multiple times on the same netlist.
         * Forbidden pins can be provided to, e.g., avoid the inclusion of logic in front of flip-flop control inputs.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] forbidden_pins - Traversal stops at pins of these types, i.e., gates reached through such a pin are not part of the result. Defaults to an empty set.
         * @param[inout] cache - An optional cache that can be used for better performance on repeated calls. Defaults to a `nullptr`.
         * @returns The next combinational gates on success, an error otherwise.
         */
        Result<std::set<Gate*>>
            get_next_combinational_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins = {}, std::unordered_map<const Net*, std::set<Gate*>>* cache = nullptr) const;

        /**
         * Find the length of shortest path (i.e., the result set with the lowest number of gates) that connects the start gate with the end gate. 
         * If there is no such path an empty optional is returned.
         * Computing only the shortest distance to a gate is faster than computing the shortest path, since it does not have to keep track of the path to reach each gate.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_gate - The gate to connect to.
         * @param[in] direction - The direction to search in. Can be PinDirection::input, PinDirection::output or PinDirection::inout to search both directions and return the shorter one.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @return An optional unsigned integer representing the shortest distance between the start and end gate on success, an error otherwise.
         */
        Result<std::optional<u32>> get_shortest_path_distance(const Gate* start_gate,
                                                              const Gate* end_gate,
                                                              const PinDirection& direction,
                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Find the shortest path (i.e., the result set with the lowest number of gates) that connects the start gate with the end gate. 
         * The gate where the search started from will be the first in the result vector, the end gate will be the last. 
         * If there is no such path an empty optional is returned. If there is more than one path with the same length only the first one is returned.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_gate - The gate to connect to.
         * @param[in] direction - The direction to search in. Can be PinDirection::input, PinDirection::output or PinDirection::inout to search both directions and return the shorter one.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @return An optional vector of gates that connect the start with end gate on success, an error otherwise.
         */
        Result<std::optional<std::vector<Gate*>>> get_shortest_path(const Gate* start_gate,
                                                                    const Gate* end_gate,
                                                                    const PinDirection& direction,
                                                                    const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                    const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Find the shortest path (i.e., the result set with the lowest number of gates) that connects the start gate
         * with any gate of the given module. The start gate will be the first in the result vector, the gate reached
         * within the module the last. If there is no such path an empty optional is returned. If there is more than
         * one path with the same length only the first one is returned. A start gate that already belongs to the
         * module yields a path consisting of that gate alone.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_module - The module to connect to. Gates of its submodules count as belonging to it.
         * @param[in] direction - The direction to search in. Can be PinDirection::input, PinDirection::output or PinDirection::inout to search both directions and return the shorter one.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @return An optional vector of gates that connect the start gate with the module on success, an error otherwise.
         */
        Result<std::optional<std::vector<Gate*>>> get_shortest_path(const Gate* start_gate,
                                                                    const Module* end_module,
                                                                    const PinDirection& direction,
                                                                    const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                    const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Find every shortest path (i.e., the result sets with the lowest number of gates) that connects the start
         * module with the end module. There may be more than one such path, so every path of that length is returned;
         * each runs from a gate of the start module to a gate of the end module. If there is no such path an empty
         * vector is returned.
         *
         * @param[in] start_module - The module to start from. Gates of its submodules count as belonging to it.
         * @param[in] end_module - The module to connect to. Gates of its submodules count as belonging to it.
         * @param[in] direction - The direction to search in. Can be PinDirection::input, PinDirection::output or PinDirection::inout to search both directions and return the shorter one.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @return A vector of the shortest paths connecting the two modules on success, an error otherwise.
         */
        Result<std::vector<std::vector<Gate*>>> get_shortest_path(const Module* start_module,
                                                                  const Module* end_module,
                                                                  const PinDirection& direction,
                                                                  const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                                  const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Find a chain of gates of the same type, starting at the given gate and following its output pins.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] input_pins - The input pins to follow. Defaults to all of them.
         * @param[in] output_pins - The output pins to follow. Defaults to all of them.
         * @param[in] filter - An optional filter a gate has to pass to be part of the chain.
         * @returns The gates of the chain in order on success, an error otherwise.
         */
        Result<std::vector<Gate*>> get_gate_chain(Gate* start_gate,
                                                  const std::vector<const GatePin*>& input_pins  = {},
                                                  const std::vector<const GatePin*>& output_pins = {},
                                                  const std::function<bool(const Gate*)>& filter = nullptr) const;

        /**
         * Find a chain of gates that repeats the given sequence of gate types, starting at the given gate.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] chain_types - The gate types the chain repeats, in order.
         * @param[in] input_pins - The input pins to follow, per gate type. Defaults to all of them.
         * @param[in] output_pins - The output pins to follow, per gate type. Defaults to all of them.
         * @param[in] filter - An optional filter a gate has to pass to be part of the chain.
         * @returns The gates of the chain in order on success, an error otherwise.
         */
        Result<std::vector<Gate*>> get_complex_gate_chain(Gate* start_gate,
                                                          const std::vector<GateType*>& chain_types,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& input_pins  = {},
                                                          const std::map<GateType*, std::vector<const GatePin*>>& output_pins = {},
                                                          const std::function<bool(const Gate*)>& filter                      = nullptr) const;

    private:
        /**
         * The breadth-first search behind every get_shortest_path overload, stopping at the first gate the given
         * condition accepts. Kept in one place so that searching for one gate and searching for any gate of a module
         * cannot drift apart.
         */
        Result<std::optional<std::vector<Gate*>>> get_shortest_path_to(const Gate* start_gate,
                                                                       const std::function<bool(const Gate*)>& is_target,
                                                                       const PinDirection& direction,
                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const;

        const Netlist& m_netlist;
    };
}    // namespace hal