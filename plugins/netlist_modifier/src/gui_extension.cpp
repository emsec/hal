#include "netlist_modifier/netlist_modifier.h"

namespace hal
{
    GuiExtensionNetlistModifier::GuiExtensionNetlistModifier()
    {
    }

    std::vector<ContextMenuContribution>
        GuiExtensionNetlistModifier::get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets)
    {
        std::vector<ContextMenuContribution> additions;

        additions.push_back({this, "modify_in_place", "Modify selected gates in place"});

        return additions;
    }

    void GuiExtensionNetlistModifier::execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets)
    {
        if (tag == "modify_in_place")
        {
            m_parent->open_popup();
        }
    }
}    // namespace hal