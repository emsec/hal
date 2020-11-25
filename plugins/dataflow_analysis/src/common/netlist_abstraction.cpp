#include "dataflow_analysis/common/netlist_abstraction.h"

#include "dataflow_analysis/utils/utils.h"
#include "dataflow_analysis/utils/utils_lsi_10k.h"
#include "dataflow_analysis/utils/utils_nangate.h"
#include "dataflow_analysis/utils/utils_ice40ultra.h"
#include "dataflow_analysis/utils/utils_xilinx_simprim.h"
#include "dataflow_analysis/utils/utils_xilinx_unisim.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace dataflow
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
            else if (nl->get_gate_library()->get_name() == "XILINX_SIMPRIM")
            {
                utils = std::make_shared<dataflow_utils::UtilsXilinxSimprim>();
            }
            else if (nl->get_gate_library()->get_name() == "ICE40ULTRA")
            {
                utils = std::make_shared<dataflow_utils::UtilsiCE40Ultra>();
            }
            else
            {
                log_error("dataflow", "no utils for gate library, you need to create the corresponding utils for this gate-library: {}", nl->get_gate_library()->get_name());
            }
        }
    }    // namespace dataflow
}    // namespace hal
