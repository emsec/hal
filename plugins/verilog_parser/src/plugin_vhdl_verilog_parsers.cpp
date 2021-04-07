#include "hal_core/netlist/hdl_parser/hdl_parser_manager.h"
#include "verilog_parser/plugin_verilog_parser.h"
#include "vhdl_verilog_parsers/hdl_parser_verilog.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VerilogParserPlugin>();
    }

    std::string VerilogParserPlugin::get_name() const
    {
        return std::string("verilog_parser");
    }

    std::string VerilogParserPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VerilogParserPlugin::on_load()
    {
        hdl_parser_manager::register_parser("Default Verilog Parser", []() { return std::make_unique<HDLParserVerilog>(); }, {".v"});
    }

    void VerilogParserPlugin::on_unload()
    {
        hdl_parser_manager::unregister_parser("Default Verilog Parser");
    }
}    // namespace hal
