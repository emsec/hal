#include "hal_core/plugin_system/plugin_interface_base.h"

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_interface_gui.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    void BasePluginInterface::initialize()
    {
    }

    void BasePluginInterface::on_load()
    {
    }

    void BasePluginInterface::on_unload()
    {
    }

    std::set<std::string> BasePluginInterface::get_dependencies() const
    {
        return {};
    }

    void BasePluginInterface::initialize_logging()
    {
        LogManager& l = LogManager::get_instance();
        l.add_channel(get_name(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

    bool BasePluginInterface::has_type(PluginInterfaceType t) const
    {
        return (t == PluginInterfaceType::base) || (t == PluginInterfaceType::cli && dynamic_cast<const CLIPluginInterface*>(this)) || (t == PluginInterfaceType::interactive_ui && dynamic_cast<const UIPluginInterface*>(this))
               || (t == PluginInterfaceType::gui && dynamic_cast<const GUIPluginInterface*>(this));
        return false;
    }

    std::vector<PluginParameter> BasePluginInterface::get_parameter() const { return std::vector<PluginParameter>(); }

    void BasePluginInterface::register_progress_indicator(std::function<void(int,const std::string&)>) {;}

    void BasePluginInterface::set_parameter(Netlist*, const std::vector<PluginParameter>&) {;}

}    // namespace hal
