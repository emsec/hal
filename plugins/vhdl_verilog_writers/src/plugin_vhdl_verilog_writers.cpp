#include "plugin_vhdl_verilog_writers.h"

#include "netlist/hdl_writer/hdl_writer_manager.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VhdlVerilogWritersPlugin>();
    }

    std::string VhdlVerilogWritersPlugin::get_name() const
    {
        return std::string("vhdl_verilog_writers");
    }

    std::string VhdlVerilogWritersPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VhdlVerilogWritersPlugin::on_load()
    {
        hdl_writer_manager::register_writer(&m_verilog_writer, {".v"});
        hdl_writer_manager::register_writer(&m_vhdl_writer, {".vhd", ".vhdl"});
    }

    void VhdlVerilogWritersPlugin::on_unload()
    {
        hdl_writer_manager::unregister_writer(&m_verilog_writer);
        hdl_writer_manager::unregister_writer(&m_vhdl_writer);
    }
}
