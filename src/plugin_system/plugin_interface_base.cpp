#include "hal_core/plugin_system/plugin_interface_base.h"

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
        LogManager::get_instance()->add_channel(get_name(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

}    // namespace hal
