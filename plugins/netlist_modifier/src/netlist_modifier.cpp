#include "netlist_modifier/netlist_modifier.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistModifierPlugin>();
    }

    NetlistModifierPlugin::NetlistModifierPlugin()
    {
        m_gui_extension = nullptr;
    }

    std::string NetlistModifierPlugin::get_name() const
    {
        return std::string("netlist-modifier");
    }

    std::string NetlistModifierPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistModifierPlugin::on_load()
    {
        m_gui_extension           = new GuiExtensionNetlistModifier;
        m_gui_extension->m_parent = this;
        m_extensions.push_back(m_gui_extension);
    }

    void NetlistModifierPlugin::on_unload()
    {
        delete_extension(m_gui_extension);
    }

    std::set<std::string> NetlistModifierPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("hal_gui");
        retval.insert("verilog_writer");
        retval.insert("hgl_writer");
        return retval;
    }

    void NetlistModifierPlugin::initialize()
    {
    }

    std::string NetlistModifierPlugin::obfuscated_gate_name(int num_in, int num_out, int num_io)
    {
        std::string retval = "UNKNOWN_" + std::to_string(num_in) + "IN_" + std::to_string(num_out) + "OUT";
        if (num_io)
            retval += "_" + std::to_string(num_io) + "IO";
        return retval;
    }
}    // namespace hal
