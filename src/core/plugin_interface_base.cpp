#include "core/plugin_interface_base.h"

#include "core/plugin_interface_cli.h"
#include "core/plugin_interface_gui.h"
#include "core/plugin_interface_interactive_ui.h"
#include "core/log.h"

namespace hal
{
    void BasePluginInterface::initialize()
    {
    }

    void BasePluginInterface::on_load() const
    {
    }

    void BasePluginInterface::on_unload() const
    {
    }

    std::set<std::string> BasePluginInterface::get_dependencies() const
    {
        return {};
    }

    void BasePluginInterface::initialize_logging() const
    {
        LogManager& l = LogManager::get_instance();
        l.add_channel(get_name(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

    bool BasePluginInterface::has_type(PluginInterfaceType t) const
    {
        return (t == PluginInterfaceType::base) || (t == PluginInterfaceType::cli && dynamic_cast<const CLIPluginInterface*>(this)) || (t == PluginInterfaceType::interactive_ui && dynamic_cast<const InteractiveUIPluginInterface*>(this))
               || (t == PluginInterfaceType::gui && dynamic_cast<const GUIPluginInterface*>(this));
        return false;
    }
}    // namespace hal
