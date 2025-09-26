#include "cross_probing/cross_probing.h"
#include "cross_probing/cross_probing_server.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<CrossProbingPlugin>();
    }

    CrossProbingPlugin::CrossProbingPlugin()
    {
        m_gui_extension = nullptr;
    }

    std::string CrossProbingPlugin::get_name() const
    {
        return std::string("cross_probing");
    }

    std::string CrossProbingPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void CrossProbingPlugin::on_load()
    {
        m_gui_extension           = new GuiExtensionCrossProbing;
        m_gui_extension->m_parent = this;
        m_extensions.push_back(m_gui_extension);
        m_server = new CrossProbingServer;
    }

    void CrossProbingPlugin::on_unload()
    {
        delete_extension(m_gui_extension);
        delete m_server;
    }

    void CrossProbingPlugin::initialize()
    {
    }

    std::set<std::string> CrossProbingPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("hal_gui");
        return retval;
    }
}    // namespace hal
