#include "hgl_writer/plugin_hgl_writer.h"

#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"
#include "hgl_writer/hgl_writer.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<HGLWriterPlugin>();
    }

    HGLWriterExtension::HGLWriterExtension()
        : FacExtensionInterface(FacExtensionInterface::FacGatelibWriter)
    {
        m_description = "Default HGL Writer";
        m_supported_file_extensions.push_back(".hgl");
        FacFactoryProvider<GateLibraryWriter>* fac = new FacFactoryProvider<GateLibraryWriter>;
        fac->m_factory = []() { return std::make_unique<HGLWriter>(); };
        factory_provider = fac;
    }

    HGLWriterPlugin::HGLWriterPlugin()
        : m_extension(nullptr)
    {;}

    std::string HGLWriterPlugin::get_name() const
    {
        return std::string("hgl_writer");
    }

    std::string HGLWriterPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void HGLWriterPlugin::on_load()
    {
        m_extension = new HGLWriterExtension;
        m_extensions.push_back(m_extension);
    }

    void HGLWriterPlugin::on_unload()
    {
        delete_extension(m_extension);
    }
}    // namespace hal
