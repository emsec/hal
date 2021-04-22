#include "vhdl_verilog_parsers/plugin_vhdl_verilog_parsers.h"

#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "vhdl_verilog_parsers/hdl_parser_verilog.h"
#include "vhdl_verilog_parsers/hdl_parser_vhdl.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VhdlVerilogParsersPlugin>();
    }

    std::string VhdlVerilogParsersPlugin::get_name() const
    {
        return std::string("vhdl_verilog_parsers");
    }

    std::string VhdlVerilogParsersPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VhdlVerilogParsersPlugin::on_load()
    {
        // netlist_parser_manager::register_parser("Default Verilog Parser", [](){return std::make_unique<HDLParserVerilog>();}, {".v"});
        netlist_parser_manager::register_parser("Default VHDL Parser", []() { return std::make_unique<HDLParserVHDL>(); }, {".vhd", ".vhdl"});
    }

    void VhdlVerilogParsersPlugin::on_unload()
    {
        netlist_parser_manager::unregister_parser("Default Verilog Parser");
        netlist_parser_manager::unregister_parser("Default VHDL Parser");
    }
}    // namespace hal
