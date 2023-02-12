#include "hal_core/plugin_system/gui_extension_interface.h"

namespace hal {
    std::vector<PluginParameter> GuiExtensionInterface::get_parameter() const { return std::vector<PluginParameter>(); }

    void GuiExtensionInterface::register_progress_indicator(std::function<void(int,const std::string&)>) {;}

    void GuiExtensionInterface::set_parameter(const std::vector<PluginParameter>&) {;}

    std::vector<ContextMenuContribution> GuiExtensionInterface::get_context_contribution(const Netlist*,
                                                                          const std::vector<u32>&,
                                                                          const std::vector<u32>&,
                                                                          const std::vector<u32>&)
    {
        return std::vector<ContextMenuContribution>();
    }

    void GuiExtensionInterface::execute_function(std::string,
                                                 Netlist*,
                                                 const std::vector<u32> &,
                                                 const std::vector<u32> &,
                                                 const std::vector<u32> &) {;}


    void GuiExtensionInterface::netlist_loaded(Netlist*) {;}

    void GuiExtensionInterface::netlist_about_to_close(Netlist*) {;}

}
