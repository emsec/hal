#include "plugin_gate_library_specific_utils.h"

#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"

// gate library specific data
#include "utils/utils_ice40ultra.h"
#include "utils/utils_lsi_10k.h"
#include "utils/utils_nangate.h"
#include "utils/utils_xilinx_simprim.h"
#include "utils/utils_xilinx_unisim.h"

#include <memory>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<GateLibrarySpecificUtils>();
    }

    std::string GateLibrarySpecificUtils::get_name() const
    {
        return std::string("gate_library_specific_utils");
    }

    std::string GateLibrarySpecificUtils::get_version() const
    {
        return std::string("0.1");
    }

    void GateLibrarySpecificUtils::initialize()
    {
    }

    std::unique_ptr<gate_library_specific_utils::Utils> GateLibrarySpecificUtils::get_gl_utils(const GateLibrary* gate_lib)
    {
        std::unique_ptr<gate_library_specific_utils::Utils> ret;

        std::string library = gate_lib->get_name();

        if (library == "XILINX_UNISIM")
        {
            ret = std::make_unique<gate_library_specific_utils::UtilsXilinxUnisim>();
        }
        else if (library == "NangateOpenCellLibrary")
        {
            ret = std::make_unique<gate_library_specific_utils::UtilsNangate>();
        }
        else if (library == "lsi_10k")
        {
            ret = std::make_unique<gate_library_specific_utils::UtilsLSI_10K>();
        }
        else if (library == "XILINX_SIMPRIM")
        {
            ret = std::make_unique<gate_library_specific_utils::UtilsXilinxSimprim>();
        }
        else if (library == "ICE40ULTRA")
        {
            ret = std::make_unique<gate_library_specific_utils::UtilsiCE40Ultra>();
        }
        else
        {
            log_error("Gate Library Specifics", "no utils for gate library, you need to create the corresponding utils for this gate-library: {}", library);
        }

        return ret;
    }

}    // namespace hal
