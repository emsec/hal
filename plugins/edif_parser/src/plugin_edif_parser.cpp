#include "edif_parser/plugin_edif_parser.h"

#include "edif_parser/edif_parser.h"
#include "hal_core/netlist/netlist_parser/netlist_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<EdifParserPlugin>();
    }

    EdifParserExtension::EdifParserExtension() : FacExtensionInterface(FacExtensionInterface::FacNetlistParser)
    {
        m_description = "Default EDIF Parser";
        m_supported_file_extensions.push_back(".edf");
        FacFactoryProvider<NetlistParser>* fac = new FacFactoryProvider<NetlistParser>;
        fac->m_factory                         = []() { return std::make_unique<EdifParser>(); };
        factory_provider                       = fac;
    }

    EdifParserPlugin::EdifParserPlugin() : m_extension(nullptr)
    {
    }

    std::string EdifParserPlugin::get_name() const
    {
        return std::string("edif_parser");
    }

    std::string EdifParserPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void EdifParserPlugin::on_load()
    {
        m_extension = new EdifParserExtension;
        m_extensions.push_back(m_extension);
    }

    void EdifParserPlugin::on_unload()
    {
        delete_extension(m_extension);
    }
}    // namespace hal
