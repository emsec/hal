#include "netlist_simulator_study/netlist_simulator_study.h"

namespace hal
{
    GuiExtensionNetlistSimulatorStudy::GuiExtensionNetlistSimulatorStudy()
    {
    }

    std::function<void(int, const std::string&)> GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function = nullptr;

    void GuiExtensionNetlistSimulatorStudy::register_progress_indicator(std::function<void(int, const std::string&)> pif)
    {
        s_progress_indicator_function = pif;
    }

    /**
     * @brief This functions adds the item to the right click menu of the HAL GUI
     * 
     * @param nl 
     * @param mods 
     * @param gates 
     * @param nets 
     * @return std::vector<ContextMenuContribution> 
     */
    std::vector<ContextMenuContribution>
        GuiExtensionNetlistSimulatorStudy::get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets)
    {
        std::vector<ContextMenuContribution> additions;

        additions.push_back({this, "sim_study_execute", "Execute simulation using selected probes"});

        return additions;
    }

    /**
     * @brief This functions handles the event if the user selects the new right click menu entry
     * 
     * @param tag 
     * @param nl 
     * @param mods 
     * @param gates 
     * @param nets 
     */
    void GuiExtensionNetlistSimulatorStudy::execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets)
    {
        if (tag == "sim_study_execute")
        {
            m_parent->open_popup();
        }
    }
}    // namespace hal