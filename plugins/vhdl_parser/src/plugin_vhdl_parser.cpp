#include "vhdl_parser/plugin_vhdl_parser.h"

#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"
#include "vhdl_parser/vhdl_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VHDLParserPlugin>();
    }

    VHDLParserExtension::VHDLParserExtension()
        : FacExtensionInterface(FacExtensionInterface::FacGatelibParser)
    {
        m_description = "Default VHDL Parser";
        m_supported_file_extensions.push_back(".vhd");
        m_supported_file_extensions.push_back(".vhdl");
        FacFactoryProvider<NetlistParser>* fac = new FacFactoryProvider<NetlistParser>;
        fac->m_factory = []() { return std::make_unique<VHDLParser>(); };
        factory_provider = fac;
    }

    VHDLParserPlugin::VHDLParserPlugin()
        : m_extension(nullptr)
    {;}

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
        m_extension = new VHDLParserExtension;
    }

    void VHDLParserPlugin::on_unload()
    {
        delete m_extension;
    }

    std::vector<AbstractExtensionInterface*> VHDLParserPlugin::get_extensions() const
    {
        return std::vector<AbstractExtensionInterface*>({m_extension});
    }
}    // namespace hal
