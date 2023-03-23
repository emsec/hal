#include "hal_core/plugin_system/plugin_interface_base.h"

#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/abstract_extension_interface.h"
#include "hal_core/utilities/log.h"


namespace hal
{
    BasePluginInterface::~BasePluginInterface()
    {
        for (AbstractExtensionInterface* aeif : m_extensions)
            delete aeif;
    }

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

    std::vector<AbstractExtensionInterface*> BasePluginInterface::get_extensions() const
    {
        return m_extensions;
    }

    void BasePluginInterface::delete_extension(AbstractExtensionInterface* aeif)
    {
        auto it = m_extensions.begin();
        while (it != m_extensions.end())
        {
            if (*it == aeif)
            {
                it = m_extensions.erase(it);
                delete aeif;
            }
            else
                ++it;
        }
    }

    void BasePluginInterface::initialize_logging()
    {
        LogManager::get_instance()->add_channel(get_name(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

}    // namespace hal
