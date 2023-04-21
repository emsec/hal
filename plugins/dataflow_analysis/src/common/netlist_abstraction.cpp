#include "dataflow_analysis/common/netlist_abstraction.h"

#include "dataflow_analysis/common/grouping.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace dataflow
    {
        NetlistAbstraction::NetlistAbstraction(Netlist* nl_arg) : nl(nl_arg)
        {
        }
    }    // namespace dataflow
}    // namespace hal
