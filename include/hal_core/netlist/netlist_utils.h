#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"

#include <set>

namespace hal
{
    class Gate;
    class Net;

    /**
     * @ingroup netlist
     */

    namespace netlist_utils
    {
        CORE_API BooleanFunction get_subgraph_function(const std::set<const Gate*> subgraph_gates, const Net* output_net);
        CORE_API std::unique_ptr<Netlist> create_deepcopy(const Netlist* nl);
    }
}    // namespace hal