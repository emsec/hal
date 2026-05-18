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

    std::vector<PluginParameter> XilinxGuiExtension::get_parameter() const
    {
        return {
            {PluginParameter::Boolean,    "split_luts",             "Split all LUT6_2 gates",          mSplitLuts           ? "true" : "false"},
            {PluginParameter::Boolean,    "split_shift_registers",  "Split all shift register gates",  mSplitShiftRegisters ? "true" : "false"},
            {PluginParameter::PushButton, "execute",                "Execute"},
        };
    }

    void XilinxGuiExtension::set_parameter(const std::vector<PluginParameter>& params)
    {
        for (const auto& p : params)
        {
            if (p.get_tagname() == "split_luts")
                mSplitLuts = (p.get_value() == "true");
            else if (p.get_tagname() == "split_shift_registers")
                mSplitShiftRegisters = (p.get_value() == "true");
        }
    }

    // -------------------------------------------------------------------------

    namespace
    {
        bool is_srl_gate(const Gate* g)
        {
            const std::string& t = g->get_type()->get_name();
            return t == "SRL16E" || t == "SRL16" || t == "SRLC16E" || t == "SRLC32E";
        }
    }

    std::vector<ContextMenuContribution> XilinxGuiExtension::get_context_contribution(
        const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets)
    {
        if (gats.size() == 1)
        {
            Gate* g = nl->get_gate_by_id(gats.front());
            if (!g)
                return {};
            if (g->get_type()->get_name() == "LUT6_2")
                return {{this, "split_lut", "Split LUT6_2"}};
            if (is_srl_gate(g))
                return {{this, "split_shift_register", "Split shift register"}};
        }
        else if (gats.size() > 1)
        {
            bool has_lut  = false;
            bool has_srl  = false;
            for (u32 id : gats)
            {
                Gate* g = nl->get_gate_by_id(id);
                if (!g) continue;
                if (g->get_type()->get_name() == "LUT6_2") has_lut = true;
                if (is_srl_gate(g))                        has_srl = true;
                if (has_lut && has_srl) break;
            }
            std::vector<ContextMenuContribution> contributions;
            if (has_lut)
                contributions.push_back({this, "split_luts_in_selection",            "Split LUT6_2 gates in selection"});
            if (has_srl)
                contributions.push_back({this, "split_shift_registers_in_selection", "Split shift registers in selection"});
            return contributions;
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
        else if (tag == "split_shift_register" && gats.size() == 1)
        {
            Gate* g = nl->get_gate_by_id(gats.front());
            if (!g)
            {
                log_error("xilinx_toolbox", "split_shift_register: gate with ID {} not found.", gats.front());
                return;
            }
            if (auto res = xilinx_toolbox::split_shift_register(g); res.is_error())
                log_error("xilinx_toolbox", "split_shift_register failed: {}", res.get_error().get());
        }
        else if (tag == "split_luts_in_selection")
        {
            std::vector<Gate*> gates;
            for (u32 id : gats)
                if (Gate* g = nl->get_gate_by_id(id); g && g->get_type()->get_name() == "LUT6_2")
                    gates.push_back(g);
            if (auto res = xilinx_toolbox::split_luts(gates); res.is_error())
                log_error("xilinx_toolbox", "split_luts_in_selection failed: {}", res.get_error().get());
        }
        else if (tag == "split_shift_registers_in_selection")
        {
            std::vector<Gate*> gates;
            for (u32 id : gats)
                if (Gate* g = nl->get_gate_by_id(id); g && is_srl_gate(g))
                    gates.push_back(g);
            if (auto res = xilinx_toolbox::split_shift_registers(gates); res.is_error())
                log_error("xilinx_toolbox", "split_shift_registers_in_selection failed: {}", res.get_error().get());
        }
        else if (tag == "execute")
        {
            if (mSplitLuts)
                if (auto res = xilinx_toolbox::split_luts(nl); res.is_error())
                    log_error("xilinx_toolbox", "split_luts failed: {}", res.get_error().get());
            if (mSplitShiftRegisters)
                if (auto res = xilinx_toolbox::split_shift_registers(nl); res.is_error())
                    log_error("xilinx_toolbox", "split_shift_registers failed: {}", res.get_error().get());
        }
    }
}    // namespace hal
