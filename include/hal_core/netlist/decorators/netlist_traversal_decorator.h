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

namespace hal
{
    /* Forward declaration */
    class Gate;
    class GatePin;
    class GateType;
    class Net;
    class Netlist;

    enum class GateTypeProperty;

    class NETLIST_API NetlistTraversalDecorator
    {
    public:
        /**
         * Construct new NetlistTraversalDecorator object.
         * 
         * @param[in] netlist - The netlist to operate on.
         */
        NetlistTraversalDecorator(const Netlist& netlist) : m_netlist{netlist} {};

        /**
         * Find predecessors or successors of a gate. If depth is set to 1 only direct predecessors/successors will be returned. 
         * Higher number of depth causes as many steps of recursive calls. 
         * If depth is set to 0 there is no limitation and the loop continues until no more predecessors/succesors are found.
         * If a filter function is given only gates matching the filter will be added to the result vector.
         * The result will not include the provided gate itself.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - True to return successors, false for Predecessors.
         * @param[in] depth - Depth of recursion.
         * @param[in] filter - User-defined filter function.
         * @return Vector of predecessor/successor gates.
         */
        std::vector<Gate*> get_next_gates(const Gate* gate, bool get_successors, int depth = 0, const std::function<bool(const Gate*)>& filter = nullptr);

        /**
         * Find predecessors or successors of a net. If depth is set to 1 only direct predecessors/successors will be returned. 
         * Higher number of depth causes as many steps of recursive calls. 
         * If depth is set to 0  there is no limitation and the loop continues until no more predecessors/succesors are found.
         * If a filter function is given, the recursion stops whenever the filter function evaluates to False. 
         * Only gates matching the filter will be added to the result vector.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - True to return successors, false for Predecessors.
         * @param[in] depth - Depth of recursion.
         * @param[in] filter - User-defined filter function.
         * @return Vector of predecessor/successor gates.
         */
        std::vector<Gate*> get_next_gates(const Net* net, bool get_successors, int depth = 0, const std::function<bool(const Gate*)>& filter = nullptr);

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
        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache, const u32 depth = 1);

        /**
         * Find all sequential predecessors or successors of a gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * The result may include the provided gate itself.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the gate.
         */
        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, const u32 depth = 1);

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
        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache, const u32 depth = 1);

        /**
         * Find all sequential predecessors or successors of a net.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the net.
         */
        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, const u32 depth = 1);

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
        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache);

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
        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties);

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
        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * Find all gates on the predecessor or successor path of a net.
         * Traverses all input or output nets until gates of the specified base types are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, the successor path is returned, otherwise the predecessor path is returned.
         * @param[in] stop_properties - Stop recursion when reaching a gate of a type with one of the specified properties.
         * @returns All gates on the predecessor or successor path of the net.
         */
        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties);

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
                                                  const std::function<bool(const Gate*)>& filter = nullptr);

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
                                                          const std::function<bool(const Gate*)>& filter = nullptr);

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
        // std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions = false);

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
        std::vector<const Gate*> get_shortest_path(const Gate* start_gate, const Gate* end_gate, bool search_both_directions = false);

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
        std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions = false);

    private:
        const Netlist& m_netlist;
    };
}    // namespace hal