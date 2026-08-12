#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_preprocessing/netlist_preprocessing.h"

#include <unordered_set>

namespace hal
{
    NetlistPreprocessingPlugin::NetlistPreprocessingPlugin()
    {
        m_extensions.push_back(new GuiExtensionNetlistPreprocessing());
    }

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistPreprocessingPlugin>();
    }

    std::string NetlistPreprocessingPlugin::get_name() const
    {
        return std::string("netlist_preprocessing");
    }

    std::string NetlistPreprocessingPlugin::get_version() const
    {
        return std::string("0.2");
    }

    std::string NetlistPreprocessingPlugin::get_description() const
    {
        return "A collection of tools to preprocess a netlist and prepare it for further analysis.";
    }

    std::set<std::string> NetlistPreprocessingPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("resynthesis");
        retval.insert("z3_utils");
        return retval;
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

    std::vector<ContextMenuContribution> GuiExtensionNetlistPreprocessing::get_context_contribution(const Netlist*, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>&)
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
            add("remove_buffers_selection", "Remove buffers from selection");
            add("unify_ff_outputs_selection", "Unify flip-flop outputs of selection");
        }
        else
        {
            add("remove_buffers_netlist", "Remove buffers from netlist");
            add("unify_ff_outputs_netlist", "Unify flip-flop outputs of netlist");
        }

        return retval;
    }

    void GuiExtensionNetlistPreprocessing::execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>&)
    {
        if (nl == nullptr)
        {
            log_warning("netlist_preprocessing", "cannot run preprocessing: no netlist loaded.");
            return;
        }

        // an empty scope makes the preprocessing functions consider the entire netlist
        std::vector<Gate*> scope;
        if (tag == "remove_buffers_selection" || tag == "unify_ff_outputs_selection")
        {
            scope = gates_from_selection(nl, mods, gats);
            if (scope.empty())
            {
                log_warning("netlist_preprocessing", "cannot run preprocessing on the selection: no gates selected.");
                return;
            }
        }

        if (tag == "remove_buffers_selection" || tag == "remove_buffers_netlist")
        {
            if (const auto res = netlist_preprocessing::remove_buffers(nl, scope); res.is_error())
            {
                log_error("netlist_preprocessing", "failed to remove buffers: {}", res.get_error().get());
            }
        }
        else if (tag == "unify_ff_outputs_selection" || tag == "unify_ff_outputs_netlist")
        {
            if (const auto res = netlist_preprocessing::unify_ff_outputs(nl, scope); res.is_error())
            {
                log_error("netlist_preprocessing", "failed to unify flip-flop outputs: {}", res.get_error().get());
            }
            else
            {
                log_info("netlist_preprocessing", "rerouted {} 'neg_state' outputs.", res.get());
            }
        }
        else
        {
            log_warning("netlist_preprocessing", "unknown context menu tag '{}'.", tag);
        }
    }
}    // namespace hal
