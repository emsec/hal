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
         * Find all sequential predecessors or successors of a specific gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * This may include the gate itself.
         * For improved performance when making extensive use of this function the same cache should be provided.
         *
         * @param[in] gate - The gate to start from
         * @param[in] get_successors - If true, sequential successors are returned, else sequential predecessors are returned
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function. You must use different caches for different values of get_successors.
         * @returns All sequential successors or predecessors of the gate.
         */
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * Find all sequential predecessors or successors of a specific net.
         * Traverses combinational logic until sequential gates are found.
         * For improved performance when making extensive use of this function the same cache should be provided.
         *
         * @param[in] net - The net to start from
         * @param[in] get_successors - If true, sequential successors are returned, else sequential predecessors are returned
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function. You must use different caches for different values of get_successors.
         * @returns All sequential successors or predecessors of the net.
         */
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache);

        /**
         * Find all sequential predecessors or successors of a specific gate.
         * Traverses combinational logic of all input or output nets until sequential gates are found.
         * This may include the gate itself.
         * If this function is used extensively, consider using the above variant with a cache.
         *
         * @param[in] gate - The gate to start from
         * @param[in] get_successors - If true, sequential successors are returned, else sequential predecessors are returned
         * @returns All sequential successors or predecessors of the gate.
         */
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors);

        /**
         * Find all sequential predecessors or successors of a specific net.
         * Traverses combinational logic until sequential gates are found.
         * If this function is used extensively, consider using the above variant with a cache.
         *
         * @param[in] net - The net to start from
         * @param[in] get_successors - If true, sequential successors are returned, else sequential predecessors are returned
         * @returns All sequential successors or predecessors of the net.
         */
        CORE_API std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors);
    }    // namespace netlist_utils
}    // namespace hal
