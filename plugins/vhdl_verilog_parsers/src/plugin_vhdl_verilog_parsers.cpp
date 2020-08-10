#include "plugin_vhdl_verilog_parsers.h"

#include "hdl_parser_verilog.h"
#include "hdl_parser_vhdl.h"

#include "netlist/hdl_parser/hdl_parser_manager.h"

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
        hdl_parser_manager::register_parser("Default Verilog Parser", [](){return std::make_unique<HDLParserVerilog>();}, {".v"});
        hdl_parser_manager::register_parser("Default VHDL Parser", [](){return std::make_unique<HDLParserVHDL>();}, {".vhd", ".vhdl"});
    }

    void VhdlVerilogParsersPlugin::on_unload()
    {
        hdl_parser_manager::unregister_parser("Default Verilog Parser");
        hdl_parser_manager::unregister_parser("Default VHDL Parser");
    }
}    // namespace hal
