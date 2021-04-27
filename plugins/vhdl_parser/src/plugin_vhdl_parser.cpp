#include "vhdl_parser/plugin_vhdl_parser.h"

#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "vhdl_parser/vhdl_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VHDLParserPlugin>();
    }

    std::string VHDLParserPlugin::get_name() const
    {
        return std::string("vhdl_parser");
    }

    std::string VHDLParserPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VHDLParserPlugin::on_load()
    {
        netlist_parser_manager::register_parser("Default VHDL Parser", []() { return std::make_unique<VHDLParser>(); }, {".vhd", ".vhdl"});
    }

    void VHDLParserPlugin::on_unload()
    {
        netlist_parser_manager::unregister_parser("Default VHDL Parser");
    }
}    // namespace hal
