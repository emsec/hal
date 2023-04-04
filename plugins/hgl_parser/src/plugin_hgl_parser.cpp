#include "hgl_parser/plugin_hgl_parser.h"

#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"
#include "hgl_parser/hgl_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<HGLParserPlugin>();
    }

    HGLParserExtension::HGLParserExtension()
        : FacExtensionInterface(FacExtensionInterface::FacGatelibParser)
    {
        m_description = "Default HGL Parser";
        m_supported_file_extensions.push_back(".hgl");
        FacFactoryProvider<GateLibraryParser>* fac = new FacFactoryProvider<GateLibraryParser>;
        fac->m_factory = []() { return std::make_unique<HGLParser>(); };
        factory_provider = fac;
    }

    HGLParserPlugin::HGLParserPlugin()
        : m_extension(nullptr)
    {;}

    std::string HGLParserPlugin::get_name() const
    {
        return std::string("hgl_parser");
    }

    std::string HGLParserPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void HGLParserPlugin::on_load()
    {
        m_extension = new HGLParserExtension;
        m_extensions.push_back(m_extension);
    }

    void HGLParserPlugin::on_unload()
    {
        delete_extension(m_extension);
    }
}    // namespace hal
