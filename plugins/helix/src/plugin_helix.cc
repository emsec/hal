#include "helix/plugin_helix.h"

#include "helix/gui_extension_helix.h"
#include "helix/helix.h"

#include <memory>
#include <string>
#include <vector>

namespace hal
{
    class AbstractExtensionInterface;
}

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<HelixPlugin>();
    }

    HelixPlugin::HelixPlugin()
    {
        m_helix = nullptr;
        m_gui_extension = nullptr;
    }

    std::string HelixPlugin::get_name() const
    {
        return std::string( "helix" );
    }

    std::string HelixPlugin::get_version() const
    {
        return std::string( "0.1" );
    }

    std::string HelixPlugin::get_description() const
    {
        return "Helix is a plugin that enables inter-process communication by sending and receiving commands via Redis "
               "Pub/Sub.";
    }

    void HelixPlugin::on_load()
    {
        m_gui_extension = new GuiExtensionHelix;
        m_gui_extension->m_parent = this;
        m_extensions.push_back( m_gui_extension );
        m_helix = helix::Helix::instance();
    }

    void HelixPlugin::on_unload()
    {
        delete_extension( m_gui_extension );
        delete m_helix;
    }

    void HelixPlugin::initialize()
    {
    }

    std::set<std::string> HelixPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert( "hal_gui" );
        return retval;
    }

    helix::Helix *HelixPlugin::get_helix()
    {
        return m_helix;
    }
}  // namespace hal
