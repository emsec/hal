#include "plugin_vhdl_verilog_parsers.h"

#include "hdl_parser_verilog.h"
#include "hdl_parser_vhdl.h"

#include "netlist/hdl_parser/hdl_parser_manager.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> get_plugin_instance()
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

    void VhdlVerilogParsersPlugin::on_load() const
    {
        m_verilog_parser = std::make_unique<HDLParserVerilog>();
        m_vhdl_parser = std::make_unique<HDLParserVHDL>();
        HDLParserManager::register_parser(m_verilog_parser.get(), {".v"});
        HDLParserManager::register_parser(m_vhdl_parser.get(), {".vhd", ".vhdl"});
    }

    void VhdlVerilogParsersPlugin::on_unload() const
    {
        HDLParserManager::unregister_parser(m_verilog_parser.get());
        HDLParserManager::unregister_parser(m_vhdl_parser.get());
    }
}    // namespace hal
