#include "xilinx_toolbox/plugin_xilinx_toolbox.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "xilinx_toolbox/preprocessing.h"

#include <unordered_set>

namespace hal
{
    XilinxToolboxPlugin::XilinxToolboxPlugin()
    {
        m_extensions.push_back(new GuiExtensionXilinxToolbox());
    }

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

    namespace
    {
        /**
         * The gates of the selected modules, including those of their submodules, together with the selected gates.
         * Duplicates are dropped, which matters when a gate is selected both directly and through a parent module.
         */
        std::vector<Gate*> gates_from_selection(Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats)
        {
            std::vector<Gate*> res;
            std::unordered_set<Gate*> seen;

            const auto collect = [&res, &seen](Gate* g) {
                if (g != nullptr && seen.insert(g).second)
                {
                    res.push_back(g);
                }
            };

            for (u32 id : gats)
            {
                collect(nl->get_gate_by_id(id));
            }

            for (u32 id : mods)
            {
                if (const Module* m = nl->get_module_by_id(id); m != nullptr)
                {
                    for (Gate* g : m->get_gates(nullptr, true))
                    {
                        collect(g);
                    }
                }
            }

            return res;
        }
    }    // namespace

    std::vector<ContextMenuContribution> GuiExtensionXilinxToolbox::get_context_contribution(const Netlist*, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>&)
    {
        std::vector<ContextMenuContribution> retval;

        const auto add = [this, &retval](const std::string& tag, const std::string& entry) {
            ContextMenuContribution cmc;
            cmc.mContributer = this;
            cmc.mTagname     = tag;
            cmc.mEntry       = entry;
            retval.push_back(cmc);
        };

        // a selection is what the user is pointing at, so do not offer to run on the entire netlist next to it
        if (!mods.empty() || !gats.empty())
        {
            add("split_luts_selection", "Split LUTs of selection");
            add("split_shift_registers_selection", "Split shift registers of selection");
        }
        else
        {
            add("split_luts_netlist", "Split LUTs of netlist");
            add("split_shift_registers_netlist", "Split shift registers of netlist");
        }

        return retval;
    }

    void GuiExtensionXilinxToolbox::execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>&)
    {
        if (nl == nullptr)
        {
            log_warning("xilinx_toolbox", "cannot run preprocessing: no netlist loaded.");
            return;
        }

        // an empty scope makes the preprocessing functions consider the entire netlist
        std::vector<Gate*> scope;
        if (tag == "split_luts_selection" || tag == "split_shift_registers_selection")
        {
            scope = gates_from_selection(nl, mods, gats);
            if (scope.empty())
            {
                log_warning("xilinx_toolbox", "cannot run preprocessing on the selection: no gates selected.");
                return;
            }
        }

        if (tag == "split_luts_selection" || tag == "split_luts_netlist")
        {
            if (const auto res = xilinx_toolbox::split_luts(nl, scope); res.is_error())
            {
                log_error("xilinx_toolbox", "failed to split LUTs: {}", res.get_error().get());
            }
            else
            {
                log_info("xilinx_toolbox", "split {} LUTs.", res.get());
            }
        }
        else if (tag == "split_shift_registers_selection" || tag == "split_shift_registers_netlist")
        {
            if (const auto res = xilinx_toolbox::split_shift_registers(nl, scope); res.is_error())
            {
                log_error("xilinx_toolbox", "failed to split shift registers: {}", res.get_error().get());
            }
            else
            {
                log_info("xilinx_toolbox", "split {} shift registers.", res.get());
            }
        }
        else
        {
            log_warning("xilinx_toolbox", "unknown context menu tag '{}'.", tag);
        }
    }
}    // namespace hal
