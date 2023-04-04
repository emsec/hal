#include "verilog_parser/plugin_verilog_parser.h"

#include "hal_core/netlist/netlist_parser/netlist_parser.h"
#include "verilog_parser/verilog_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VerilogParserPlugin>();
    }

    VerilogParserExtension::VerilogParserExtension()
        : FacExtensionInterface(FacExtensionInterface::FacNetlistParser)
    {
        m_description = "Default Verilog Parser";
        m_supported_file_extensions.push_back(".v");
        FacFactoryProvider<NetlistParser>* fac = new FacFactoryProvider<NetlistParser>;
        fac->m_factory = []() { return std::make_unique<VerilogParser>(); };
        factory_provider = fac;
    }

    VerilogParserPlugin::VerilogParserPlugin()
        : m_extension(nullptr)
    {;}

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
        m_extension = new VerilogParserExtension;
        m_extensions.push_back(m_extension);
    }

    void VerilogParserPlugin::on_unload()
    {
        delete_extension(m_extension);
    }
}    // namespace hal
