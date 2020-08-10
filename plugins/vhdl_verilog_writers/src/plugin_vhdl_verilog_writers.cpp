#include "plugin_vhdl_verilog_writers.h"

#include "hdl_writer_verilog.h"
#include "hdl_writer_vhdl.h"
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
        hdl_writer_manager::register_writer("Default Verilog Writer", []() { return std::make_unique<HDLWriterVerilog>(); }, {".v"});
        hdl_writer_manager::register_writer("Default VHDL Writer", []() { return std::make_unique<HDLWriterVHDL>(); }, {".vhd", ".vhdl"});
    }

    void VhdlVerilogWritersPlugin::on_unload()
    {
        hdl_writer_manager::unregister_writer("Default Verilog Writer");
        hdl_writer_manager::unregister_writer("Default VHDL Writer");
    }
}    // namespace hal
