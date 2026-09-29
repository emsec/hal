#include "vhdl_parser_old/plugin_vhdl_parser_old.h"


namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VHDLParserOldPlugin>();
    }

    std::string VHDLParserOldPlugin::get_name() const
    {
        return std::string("vhdl_parser_old");
    }

    std::string VHDLParserOldPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string VHDLParserOldPlugin::get_description() const
    {
        return std::string("Legacy VHDL netlist parser, replaced by vhdl_parser; registers no file extension");
    }
}    // namespace hal
