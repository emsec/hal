#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"

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
    }
}    // namespace hal