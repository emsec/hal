#include "common/netlist_abstraction.h"

#include "core/log.h"
#include "netlist/netlist.h"
#include "utils/utils.h"
#include "utils/utils_lsi_10k.h"
#include "utils/utils_nangate.h"
#include "utils/utils_xilinx_unisim.h"

namespace hal
{
    NetlistAbstraction::NetlistAbstraction(Netlist* nl_arg) : nl(nl_arg)
    {
        std::string library = nl->get_gate_library()->get_name();

        if (nl->get_gate_library()->get_name() == "XILINX_UNISIM")
        {
            utils = std::make_shared<dataflow_utils::UtilsXilinxUnisim>();
        }
        else if (nl->get_gate_library()->get_name() == "NangateOpenCellLibrary")
        {
            utils = std::make_shared<dataflow_utils::UtilsNangate>();
        }
        else if (nl->get_gate_library()->get_name() == "lsi_10k")
        {
            utils = std::make_shared<dataflow_utils::UtilsLSI_10K>();
        }
        else
        {
            log_error("dataflow", "no utils for gate library, you need to create the corresponding utils for this gate-library: {}", nl->get_gate_library()->get_name());
        }
    }
}    // namespace hal
