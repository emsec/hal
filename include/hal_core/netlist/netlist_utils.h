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
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    class Gate;
    class Net;

    /**
     * @ingroup netlist
     */

    namespace netlist_utils
    {
        /**
         * \deprecated
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the given net.
         * The variables of the resulting Boolean function are made up of the IDs of the nets that influence the output ('net_[ID]').
         * Utilizes a cache for speedup on consecutive calls.
         *
         * @param[in] net - The net for which to generate the Boolean function.
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function.
         * @returns The combined Boolean function of the subgraph on success, an error otherwise.
         */
        [[deprecated("Will be removed in a future version, use SubgraphNetlistDecorator::get_subgraph_function instead.")]] CORE_API Result<BooleanFunction>
            get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache);

        /**
         * \deprecated
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the given net.
         * The variables of the resulting Boolean function are made up of the IDs of the nets that influence the output ('net_[ID]').
         *
         * @param[in] net - The net for which to generate the Boolean function.
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @returns The combined Boolean function of the subgraph on success, an error otherwise.
         */
        [[deprecated("Will be removed in a future version, use SubgraphNetlistDecorator::get_subgraph_function instead.")]] CORE_API Result<BooleanFunction>
            get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates);

        /**
         * \deprecated
         * Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.
         *
         * @param[in] nl - The netlist to copy.
         * @returns The deep copy of the netlist.
         */
        [[deprecated("Will be removed in a future version, use Netlist::copy instead.")]] CORE_API std::unique_ptr<Netlist> copy_netlist(const Netlist* nl);

        /**
         * Get the FF dependency matrix of a netlist.
         *
         * @param[in] nl - The netlist to extract the dependency matrix from.
         * @returns A pair consisting of std::map<u32, Gate*>, which includes the mapping from the original gate
         *          IDs to the ones in the matrix, and a std::vector<std::vector<int>, which is the ff dependency matrix
         */
        std::pair<std::map<u32, Gate*>, std::vector<std::vector<int>>> get_ff_dependency_matrix(const Netlist* nl);

        /**
         * \deprecated
         * Get a deep copy of an entire partial netlist including all of its gates, nets, excluding modules and groupings.
         *
         * @param[in] nl - The netlist consisting of the subgraph.
         * @param[in] subgraph_gates - The gates the subgraph is supposed to consist of.
         * @returns The deep copy of the netlist.
         */
        [[deprecated("Will be removed in a future version, use SubgraphNetlistDecorator::copy_subgraph_netlist instead.")]] CORE_API std::unique_ptr<Netlist>
            get_partial_netlist(const Netlist* nl, const std::vector<const Gate*>& subgraph_gates);

        /**
         * \deprecated
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
        [[deprecated("Will be removed in a future version, use NetlistTraversalDecorator::get_next_matching_gates_until_depth instead.")]] CORE_API std::vector<Gate*>
            get_next_gates(const Gate* gate, bool get_successors, int depth = 0, const std::function<bool(const Gate*)>& filter = nullptr);

        /**
         * \deprecated
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
        [[deprecated("Will be removed in a future version, use NetlistTraversalDecorator::get_next_matching_gates_until_depth instead.")]] CORE_API std::vector<Gate*>
            get_next_gates(const Net* net, bool get_successors, int depth = 0, const std::function<bool(const Gate*)>& filter = nullptr);

        /**
         * \deprecated
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
        [[deprecated("Will be removed in a future version, use NetlistTraversalDecorator::get_next_sequential_gates instead.")]] CORE_API std::vector<Gate*>
            get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * \deprecated
         * Find all sequential predecessors or successors of a gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * The result may include the provided gate itself.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the gate.
         */
        [[deprecated("Will be removed in a future version, use NetlistTraversalDecorator::get_next_sequential_gates instead.")]] CORE_API std::vector<Gate*>
            get_next_sequential_gates(const Gate* gate, bool get_successors);

        /**
         * \deprecated
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
        [[deprecated("Will be removed in a future version, use NetlistTraversalDecorator::get_next_sequential_gates instead.")]] CORE_API std::vector<Gate*>
            get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * \deprecated
         * Find all sequential predecessors or successors of a net.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the net.
         */
        [[deprecated("Will be removed in a future version, use NetlistTraversalDecorator::get_next_sequential_gates instead.")]] CORE_API std::vector<Gate*>
            get_next_sequential_gates(const Net* net, bool get_successors);

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
        CORE_API std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache);

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
        CORE_API std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties);

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
        CORE_API std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * Find all gates on the predecessor or successor path of a net.
         * Traverses all input or output nets until gates of the specified base types are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, the successor path is returned, otherwise the predecessor path is returned.
         * @param[in] stop_properties - Stop recursion when reaching a gate of a type with one of the specified properties.
         * @returns All gates on the predecessor or successor path of the net.
         */
        CORE_API std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties);

        /**
         * Get the nets that are connected to a subset of pins of the specified gate.
         * 
         * @param[in] gate - The gate.
         * @param[in] pins - The targeted pins.
         * @returns A vector of nets connected to the pins.
         */
        CORE_API std::vector<Net*> get_nets_at_pins(Gate* gate, std::vector<GatePin*> pins);

        /**
         * \deprecated
         * Remove all buffer gates from the netlist and connect their fan-in to their fan-out nets.
         * If enabled, analyzes every gate's inputs and removes fixed '0' or '1' inputs from the Boolean function.
         * 
         * @param[in] netlist - The target netlist.
         * @param[in] analyze_inputs - Set `true` to dynamically analyze the inputs, `false` otherwise.
         * @returns The number of removed buffers on success, an error otherwise.
         */
        [[deprecated("Will be removed in a future version, use NetlistPreprocessingPlugin::remove_buffers instead.")]] CORE_API Result<u32> remove_buffers(Netlist* netlist,
                                                                                                                                                           bool analyze_inputs = false);

        /**
         * \deprecated
         * Remove all LUT fan-in endpoints that are not present within the Boolean function of the output of a gate.
         * 
         * @param[in] netlist - The target netlist.
         * @returns The number of removed endpoints on success, an error otherwise.
         */
        [[deprecated("Will be removed in a future version, use NetlistPreprocessingPlugin::remove_unused_lut_inputs instead.")]] CORE_API Result<u32> remove_unused_lut_endpoints(Netlist* netlist);

        /**
         * Returns all nets that are considered to be common inputs to the provided gates.
         * A threshold value can be provided to specify the number of gates a net must be connected to in order to be classified as a common input.
         * If the theshold value is set to 0, a net must be input to all gates to be considered a common input.
         * 
         * @param[in] gates - The gates.
         * @param[in] threshold - The threshold value, defaults to 0.
         * @returns The common input nets.
         */
        // TODO move to SubgraphNetlistDecorator
        CORE_API std::vector<Net*> get_common_inputs(const std::vector<Gate*>& gates, u32 threshold = 0);

        /**
         * \deprecated
         * Replace the given gate with a gate of the specified gate type.
         * A map from old to new pins must be provided in order to correctly connect the gates inputs and outputs.
         * A pin can be omitted if no connection at that pin is desired.
         * 
         * @param[in] gate - The gate to be replaced.
         * @param[in] target_type - The gate type of the replacement gate.
         * @param[in] pin_map - A map from old to new pins.
         * @returns Ok on success, an error otherwise.
         */
        [[deprecated("Will be removed in a future version, use NetlistModificationDecorator::replace_gate instead.")]] CORE_API Result<std::monostate>
            replace_gate(Gate* gate, GateType* target_type, std::map<GatePin*, GatePin*> pin_map);

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
        CORE_API Result<std::vector<Gate*>> get_gate_chain(Gate* start_gate,
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
        CORE_API Result<std::vector<Gate*>> get_complex_gate_chain(Gate* start_gate,
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
        CORE_API std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions = false);

        /**
         * Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start gate with any gate from the given module.
         * The gate where the search started from will be the first in the result vector, the end gate will be the last.
         * If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.
         *
         * @param[in] start_gate - The gate to start from.
         * @param[in] end_module - The module to connect to.
         * @param[in] search_both_directions - True to additionally check whether a shorter path from end to start exists, false otherwise.
         * @return A vector of gates that connect the start with end gate (possibly in reverse order).
         */
        CORE_API std::vector<Gate*> get_shortest_path(Gate* start_gate, Module* end_module, bool forward_direction);


        /**
         * Find the shortest path (i.e., theresult set with the lowest number of gates) that connects the start module with the target module.
         * There might be more than one connection thus a vector of connecting gate vectors is returned. In each connecting gate vector
         *gate where the search started from will be the first, the end gate will be the last.
         * If there is no such path an empty vector is returned. If there is more than one path with the same length only the first one is returned.
         *
         * @param[in] start_module - The module to start from.
         * @param[in] end_module - The module to connect to.
         * @return A vector of connecting vectors with gates that connect the start with end gate.
         */
        CORE_API std::vector<std::vector<Gate*> > get_shortest_path(Module* start_module, Module* end_module);
    }    // namespace netlist_utils
}    // namespace hal
