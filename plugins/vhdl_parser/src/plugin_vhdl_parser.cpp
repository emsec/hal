#include "vhdl_parser/plugin_vhdl_parser.h"

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

    std::string VHDLParserPlugin::get_description() const
    {
        return std::string("VHDL netlist parser built on the netlist IR");
    }
}    // namespace hal
