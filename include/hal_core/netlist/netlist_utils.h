#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"

#include <unordered_set>

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
         * Get the combined Boolean function of a specific net, considering an entire subgraph.<br>
         * In other words, the Boolean functions of the subgraph gates that influence the target net are combined to one function.<br>
         * The variables of the resulting Boolean function are the net IDs of the nets that influence the output.
         * For high performance when used extensively, a cache is employed.
         *
         * @param[in] net - The net for which to generate the Boolean function.
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function.
         * @returns The combined Boolean function of the subgraph.
         */
        CORE_API BooleanFunction get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates, std::unordered_map<u32, BooleanFunction>& cache);

        /**
         * Get the combined Boolean function of a specific net, considering an entire subgraph.<br>
         * In other words, the Boolean functions of the subgraph gates that influence the target net are combined to one function.<br>
         * The variables of the resulting Boolean function are the net IDs of the nets that influence the output.
         * If this function is used extensively, consider using the above variant with a cache.
         *
         * @param[in] net - The net for which to generate the Boolean function.
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @returns The combined Boolean function of the subgraph.
         */
        CORE_API BooleanFunction get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates);

        /**
         * Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.
         *
         * @param[in] nl - The netlist to copy.
         * @returns The deep copy of the netlist.
         */
        CORE_API std::unique_ptr<Netlist> copy_netlist(const Netlist* nl);

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
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * Find all sequential predecessors or successors of a gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * The result may include the provided gate itself.
         *
         * @param[in] gate - The initial gate.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the gate.
         */
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors);

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
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * Find all sequential predecessors or successors of a net.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         *
         * @param[in] net - The initial net.
         * @param[in] get_successors - If true, sequential successors are returned, otherwise sequential predecessors are returned.
         * @returns All sequential successors or predecessors of the net.
         */
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors);

        /**
         * Get the nets that are connected to a subset of pins of the specified gate.
         * 
         * @param[in] gate - The gate.
         * @param[in] pins - The targeted pins.
         * @param[in] is_inputs - True to look for fan-in nets, false for fan-out.
         * @returns The set of nets connected to the pins.
         */
        std::unordered_set<Net*> get_nets_at_pins(Gate* gate, std::unordered_set<std::string> pins, bool is_inputs);

        /**
         * Remove all buffer gates from the netlist and connect their fan-in to their fan-out nets.
         * 
         * @param[in] netlist - The target netlist.
         * @param[in] check_luts - If true, check Boolean functions of LUTs and remove buffer LUTs.
         */
        void remove_buffers(Netlist* netlist, bool check_luts = false);

        /**
         * Remove all LUT fan-in endpoints that are not present within the Boolean function of the output of a gate.
         * 
         * @param[in] netlist - The target netlist.
         */
        void remove_unused_lut_endpoints(Netlist* netlist);
    }    // namespace netlist_utils
}    // namespace hal
