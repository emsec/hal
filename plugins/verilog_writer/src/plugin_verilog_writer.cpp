#include "verilog_writer/plugin_verilog_writer.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "verilog_writer/verilog_writer.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VerilogWriterPlugin>();
    }

    std::string VerilogWriterPlugin::get_name() const
    {
        return std::string("verilog_writer");
    }

    std::string VerilogWriterPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VerilogWriterPlugin::on_load()
    {
        netlist_writer_manager::register_writer("Default Verilog Writer", []() { return std::make_unique<VerilogWriter>(); }, {".v"});
    }

    void VerilogWriterPlugin::on_unload()
    {
        netlist_writer_manager::unregister_writer("Default Verilog Writer");
    }
}    // namespace hal
