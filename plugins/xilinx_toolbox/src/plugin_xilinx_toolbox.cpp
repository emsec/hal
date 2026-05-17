#include "xilinx_toolbox/plugin_xilinx_toolbox.h"
#include "xilinx_toolbox/preprocessing.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<XilinxToolboxPlugin>();
    }

    std::string XilinxToolboxPlugin::get_name() const
    {
        return std::string("xilinx_toolbox");
    }

    std::string XilinxToolboxPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string XilinxToolboxPlugin::get_description() const
    {
        return "A collection of functions specifically designed to operate on Xilinx FPGA netlists.";
    }

    std::set<std::string> XilinxToolboxPlugin::get_dependencies() const
    {
        return {};
    }

    std::vector<AbstractExtensionInterface*> XilinxToolboxPlugin::get_extensions() const
    {
        return {&mGuiExtension};
    }

    // -------------------------------------------------------------------------

    std::vector<ContextMenuContribution> XilinxGuiExtension::get_context_contribution(
        const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets)
    {
        if (gats.size() == 1)
        {
            Gate* g = nl->get_gate_by_id(gats.front());
            if (g && g->get_type()->get_name() == "LUT6_2")
                return {{this, "split_lut", "Split LUT6_2"}};
        }
        return {};
    }

    void XilinxGuiExtension::execute_function(
        std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets)
    {
        if (tag == "split_lut" && gats.size() == 1)
        {
            Gate* g = nl->get_gate_by_id(gats.front());
            if (!g)
            {
                log_error("xilinx_toolbox", "split_lut: gate with ID {} not found.", gats.front());
                return;
            }
            if (auto res = xilinx_toolbox::split_lut(g); res.is_error())
                log_error("xilinx_toolbox", "split_lut failed: {}", res.get_error().get());
        }
    }
}    // namespace hal
