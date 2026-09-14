#include "verilog_parser/plugin_verilog_parser.h"

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

    std::string VerilogParserPlugin::get_description() const
    {
        return std::string("Verilog netlist parser built on the netlist IR");
    }
}    // namespace hal
