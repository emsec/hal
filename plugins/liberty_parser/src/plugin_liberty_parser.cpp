#include "liberty_parser/plugin_liberty_parser.h"

#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "liberty_parser/liberty_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<LibertyParserPlugin>();
    }

    LibertyParserExtension::LibertyParserExtension()
        : FacExtensionInterface(FacExtensionInterface::FacGatelibParser)
    {
        m_description = "Default Liberty Parser";
        m_supported_file_extensions.push_back(".lib");
        FacFactoryProvider<GateLibraryParser>* fac = new FacFactoryProvider<GateLibraryParser>;
        fac->m_factory = []() { return std::make_unique<LibertyParser>(); };
        factory_provider = fac;
    }

    LibertyParserPlugin::LibertyParserPlugin()
        : m_extension(nullptr)
    {;}

    std::string LibertyParserPlugin::get_name() const
    {
        return std::string("liberty_parser");
    }

    std::string LibertyParserPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void LibertyParserPlugin::on_load()
    {
        m_extension = new LibertyParserExtension;
    }

    void LibertyParserPlugin::on_unload()
    {
        delete m_extension;
    }

    std::vector<AbstractExtensionInterface*> LibertyParserPlugin::get_extensions() const
    {
        return std::vector<AbstractExtensionInterface*>({m_extension});
    }
}    // namespace hal
