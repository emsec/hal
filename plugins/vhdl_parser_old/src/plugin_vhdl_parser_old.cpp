#include "vhdl_parser_old/plugin_vhdl_parser_old.h"

#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"
#include "vhdl_parser_old/vhdl_parser_old.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VHDLParserOldPlugin>();
    }

    VHDLParserOldExtension::VHDLParserOldExtension()
        : FacExtensionInterface(FacExtensionInterface::FacNetlistParser)
    {
        m_description = "Legacy VHDL parser, replaced by vhdl_parser";
        m_supported_file_extensions.push_back(".vhd");
        m_supported_file_extensions.push_back(".vhdl");
        FacFactoryProvider<NetlistParser>* fac = new FacFactoryProvider<NetlistParser>;
        fac->m_factory = []() { return std::make_unique<VHDLParserOld>(); };
        factory_provider = fac;
    }

    VHDLParserOldPlugin::VHDLParserOldPlugin()
        : m_extension(nullptr)
    {;}

    std::string VHDLParserOldPlugin::get_name() const
    {
        return std::string("vhdl_parser_old");
    }

    std::string VHDLParserOldPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VHDLParserOldPlugin::on_load()
    {
        m_extension = new VHDLParserOldExtension;
        m_extensions.push_back(m_extension);
    }

    void VHDLParserOldPlugin::on_unload()
    {
        delete_extension(m_extension);
    }
}    // namespace hal
