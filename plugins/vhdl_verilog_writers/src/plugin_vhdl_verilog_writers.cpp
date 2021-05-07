#include "vhdl_verilog_writers/plugin_vhdl_verilog_writers.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "vhdl_verilog_writers/verilog_writer.h"
#include "vhdl_verilog_writers/vhdl_writer.h"

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
        netlist_writer_manager::register_writer("Default Verilog Writer", []() { return std::make_unique<VerilogWriter>(); }, {".v"});
        netlist_writer_manager::register_writer("Default VHDL Writer", []() { return std::make_unique<VHDLWriter>(); }, {".vhd", ".vhdl"});
    }

    void VhdlVerilogWritersPlugin::on_unload()
    {
        netlist_writer_manager::unregister_writer("Default Verilog Writer");
        netlist_writer_manager::unregister_writer("Default VHDL Writer");
    }
}    // namespace hal
