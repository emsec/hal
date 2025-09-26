#include "cross_probing/cross_probing.h"
#include "cross_probing/cross_probing_control.h"
#include <QApplication>
#include <QLocalServer>

namespace hal
{
    GuiExtensionCrossProbing::GuiExtensionCrossProbing()
    {;}

    std::function<void(int, const std::string&)> GuiExtensionCrossProbing::s_progress_indicator_function = nullptr;

    void GuiExtensionCrossProbing::register_progress_indicator(std::function<void(int, const std::string&)> pif)
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
        GuiExtensionCrossProbing::get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets)
    {
        std::vector<ContextMenuContribution> additions;

        additions.push_back({this, "cross_probing_execute", "Open pipe for external HAL control."});

        return additions;
    }

    std::vector<PluginParameter> GuiExtensionCrossProbing::get_parameter() const
    {
        std::vector<PluginParameter> retval;
        retval.push_back(PluginParameter(PluginParameter::Label,
                                         "help",
                                         "",
                                         "Press 'Run' to open a window that simulates\n"
                                         "an external process communicating with HAL via\n"
                                         "local socket (named pipe in Linux).\n\n"
                                         "This demo requires the FSM project to be open\n"
                                         "and can be run only once per HAL session.\n\n"
                                         "It is recommended to interfere with the\n"
                                         "actions issued from external process\n"
                                         "as little as possible ;-)\n\n"));
        retval.push_back(PluginParameter(PluginParameter::PushButton, "run", "Run"));
        return retval;
    }

    void GuiExtensionCrossProbing::set_parameter(const std::vector<PluginParameter>& params)
    {
        for (const PluginParameter& pp : params)
        {
            // will launch window simulating external process
            if (pp.get_tagname() == "run")
            {
                CrossProbingControl* cpc = new CrossProbingControl(QApplication::activeWindow());
                cpc->show();
            }
        }
    }
 }    // namespace hal
