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

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
{
    /* Forward declaration */
    class Endpoint;
    class Gate;
    class GatePin;
    class GateType;
    class Net;
    class Netlist;

    enum class GateTypeProperty;
    enum class PinType;

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
         * Find all sequential predecessors or successors of a gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * The result may include the provided gate itself.
         * The use of the this cached version is recommended in case of extensive usage to improve performance. 
         * The cache will be filled by this function and should initially be provided empty.
         * Different caches for different values of get_successors shall be used.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @param[inout] cache - The cache. 
         * @returns All sequential successors or predecessors of the gate.
         */
        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache, const u32 depth = 1) const;

        /**
         * Find all sequential predecessors or successors of a gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * The result may include the provided gate itself.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the gate.
         */
        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, const u32 depth = 1) const;

        /**
         * Find all sequential predecessors or successors of a net.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * The use of the cache is recommended in case of extensive usage of this function. 
         * The cache will be filled by this function and should initially be provided empty.
         * Different caches for different values of get_successors shall be used.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @param[inout] cache - The cache. 
         * @returns All sequential successors or predecessors of the net.
         */
        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache, const u32 depth = 1) const;

        /**
         * Find all sequential predecessors or successors of a net.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the net.
         */
        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, const u32 depth = 1) const;

        /**
         * Find all gates on the predecessor or successor path of a gate.
         * Traverses all input or output nets until gates of the specified base types are found.
         * The result may include the provided gate itself.
         * The use of the this cached version is recommended in case of extensive usage to improve performance. 
         * The cache will be filled by this function and should initially be provided empty.
         * Different caches for different values of get_successors shall be used.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, the successor path is returned, otherwise the predecessor path is returned.
         * @param[in] stop_properties - Stop recursion when reaching a gate of a type with one of the specified properties.
         * @param[inout] cache - The cache. 
         * @returns All gates on the predecessor or successor path of the gate.
         */
        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache) const;

        /**
         * Find all gates on the predeccessor or successor path of a gate.
         * Traverses all input or output nets until gates of the specified base types are found.
         * The result may include the provided gate itself.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, the successor path is returned, otherwise the predecessor path is returned.
         * @param[in] stop_properties - Stop recursion when reaching a gate of a type with one of the specified properties.
         * @returns All gates on the predecessor or successor path of the gate.
         */
        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties) const;

        /**
         * Find all gates on the predecessor or successor path of a net.
         * Traverses all input or output nets until gates of the specified base types are found.
         * The use of the this cached version is recommended in case of extensive usage to improve performance. 
         * The cache will be filled by this function and should initially be provided empty.
         * Different caches for different values of get_successors shall be used.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, the successor path is returned, otherwise the predecessor path is returned.
         * @param[in] stop_properties - Stop recursion when reaching a gate of a type with one of the specified properties.
         * @param[inout] cache - The cache. 
         * @returns All gates on the predecessor or successor path of the net.
         */
        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache) const;

        /**
         * Find all gates on the predecessor or successor path of a net.
         * Traverses all input or output nets until gates of the specified base types are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, the successor path is returned, otherwise the predecessor path is returned.
         * @param[in] stop_properties - Stop recursion when reaching a gate of a type with one of the specified properties.
         * @returns All gates on the predecessor or successor path of the net.
         */
        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties) const;

        /**
         * Find a sequence of identical gates that are connected via the specified input and output pins.
         * The start gate may be any gate within a such a sequence, it is not required to be the first or the last gate.
         * If input and/or output pins are specified, the gates must be connected through one of the input pins and/or one of the output pins.
         * The optional filter is evaluated on every gate such that the result only contains gates matching the specified condition.
         * 
         * @param[in] start_gate - The gate at which to start the chain detection.
         * @param[in] input_pins - The input pins through which the gates must be connected. Defaults to an empty vector.
         * @param[in] output_pins - The output pins through which the gates must be connected. Defaults to an empty vector.
         * @param[in] filter - An optional filter function to be evaluated on each gate.
         * @returns A vector of gates that form a chain on success, an error otherwise.
         */
        Result<std::vector<Gate*>> get_gate_chain(Gate* start_gate,
                                                  const std::vector<const GatePin*>& input_pins  = {},
                                                  const std::vector<const GatePin*>& output_pins = {},
                                                  const std::function<bool(const Gate*)>& filter = nullptr) const;

        /**
         * Find a sequence of gates (of the specified sequence of gate types) that are connected via the specified input and output pins.
         * The start gate may be any gate within a such a sequence, it is not required to be the first or the last gate.
         * However, the start gate must be of the first gate type within the repeating sequence.
         * If input and/or output pins are specified for a gate type, the gates must be connected through one of the input pins and/or one of the output pins.
         * The optional filter is evaluated on every gate such that the result only contains gates matching the specified condition.
         * 
         * @param[in] start_gate - The gate at which to start the chain detection.
         * @param[in] chain_types - The sequence of gate types that is expected to make up the gate chain.
         * @param[in] input_pins - The input pins (of every gate type of the sequence) through which the gates must be connected.
         * @param[in] output_pins - The output pins (of every gate type of the sequence) through which the gates must be connected.
         * @param[in] filter - An optional filter function to be evaluated on each gate.
         * @returns A vector of gates that form a chain on success, an error otherwise.
         */
        Result<std::vector<Gate*>> get_complex_gate_chain(Gate* start_gate,
                                                          const std::vector<GateType*>& chain_types,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& input_pins,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& output_pins,
                                                          const std::function<bool(const Gate*)>& filter = nullptr) const;

        /**
         * Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start gate with the end gate. 
         * The gate where the search started from will be the first in the result vector, the end gate will be the last. 
         * If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_gate - The gate to connect to.
         * @param[in] search_both_directions - True to additionally check whether a shorter path from end to start exists, false otherwise.
         * @return A vector of gates that connect the start with end gate (possibly in reverse order).
         */
        // std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions = false) const;

        /**
         * Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start gate with the end gate. 
         * The gate where the search started from will be the first in the result vector, the end gate will be the last. 
         * If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_gate - The gate to connect to.
         * @param[in] search_both_directions - True to additionally check whether a shorter path from end to start exists, false otherwise.
         * @return A vector of gates that connect the start with end gate (possibly in reverse order).
         */
        std::vector<const Gate*> get_shortest_path(const Gate* start_gate, const Gate* end_gate, bool search_both_directions = false) const;

        /**
         * Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start gate with the end gate. 
         * The gate where the search started from will be the first in the result vector, the end gate will be the last. 
         * If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_gate - The gate to connect to.
         * @param[in] search_both_directions - True to additionally check whether a shorter path from end to start exists, false otherwise.
         * @return A vector of gates that connect the start with end gate (possibly in reverse order).
         */
        std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions = false) const;

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

        /**
         * Starting from the given net, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Stop traversal if (1) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (2) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * The target_gate_filter may be ommitted in which case all traversed gates will be returned.
         * Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted as well.
         * 
         * @param[in] net - Start net.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition.
         */
        Result<std::set<Gate*>> get_subgraph_gates(const Net* net,
                                                   bool successors,
                                                   const std::function<bool(const Gate*)>& target_gate_filter                                 = nullptr,
                                                   const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                   const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

        /**
         * Starting from the given gate, traverse the netlist and return only the successor/predecessor gates for which the `target_gate_filter` evaluates to `true`.
         * Stop traversal if (1) the `exit_endpoint_filter` evaluates to `false` on a fan-in/out endpoint (i.e., when exiting the current gate during traversal), or (2) the `entry_endpoint_filter` evaluates to `false` on a successor/predecessor endpoint (i.e., when entering the next gate during traversal).
         * The target_gate_filter may be ommitted in which case all traversed gates will be returned.
         * Both `entry_endpoint_filter` and the `exit_endpoint_filter` may be omitted as well.
         * 
         * @param[in] gate - Start gate.
         * @param[in] successors - Set `true` to get successors, set `false` to get predecessors.
         * @param[in] target_gate_filter - Filter condition that must be met for the target gates.
         * @param[in] exit_endpoint_filter - Filter condition that determines whether to stop traversal on a fan-in/out endpoint.
         * @param[in] entry_endpoint_filter - Filter condition that determines whether to stop traversal on a successor/predecessor endpoint.
         * @returns The next gates fulfilling the target gate filter condition.
         */
        Result<std::set<Gate*>> get_subgraph_gates(const Gate* gate,
                                                   bool successors,
                                                   const std::function<bool(const Gate*)>& target_gate_filter                                 = nullptr,
                                                   const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter  = nullptr,
                                                   const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter = nullptr) const;

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