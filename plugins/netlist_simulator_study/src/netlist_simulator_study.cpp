#include "netlist_simulator_study/netlist_simulator_study.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistSimulatorStudyPlugin>();
    }

    NetlistSimulatorStudyPlugin::NetlistSimulatorStudyPlugin()
    {
        m_gui_extension = nullptr;
    }

    std::string NetlistSimulatorStudyPlugin::get_name() const
    {
        return std::string("netlist_simulator_study");
    }

    std::string NetlistSimulatorStudyPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistSimulatorStudyPlugin::on_load()
    {
        m_gui_extension           = new GuiExtensionNetlistSimulatorStudy;
        m_gui_extension->m_parent = this;
        m_extensions.push_back(m_gui_extension);
    }

    void NetlistSimulatorStudyPlugin::on_unload()
    {
        delete_extension(m_gui_extension);
    }

    void NetlistSimulatorStudyPlugin::initialize()
    {
    }

    std::set<std::string> NetlistSimulatorStudyPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        retval.insert("hal_gui");
        retval.insert("waveform_viewer");
        retval.insert("verilator");
        retval.insert("liberty_parser");
        retval.insert("verilog_parser");
        return retval;
    }
}    // namespace hal
