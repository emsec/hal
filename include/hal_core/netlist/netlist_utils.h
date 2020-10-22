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
         * Get the combined Boolean function of a subgraph of the netlist that is defined by a set of gates and an output net.<br>
         * The gates within the net should be connected and the given output net should represent the targeted output of the subgraph.<br>
         * The resulting Boolean function uses the net IDs of the automatically identified input nets as variables.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph.
         * @param[in] output_net - The output net for which to generate the Boolean function.
         * @returns The combined Boolean function of the subgraph.
         */
        CORE_API BooleanFunction get_subgraph_function(const std::unordered_set<const Gate*>& subgraph_gates, const Net* output_net);

        /**
         * Get a deep copy of an entire netlist including all of its gates, nets, modules, and groupings.
         * 
         * @param[in] nl - The netlist to copy.
         * @returns The deep copy of the netlist.
         */
        CORE_API std::unique_ptr<Netlist> copy_netlist(const Netlist* nl);
    }    // namespace netlist_utils
}    // namespace hal