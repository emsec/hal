#include "gexf_writer/plugin_gexf_writer.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "gexf_writer/gexf_writer.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<GexfWriterPlugin>();
    }

    GexfWriterExtension::GexfWriterExtension()
        : FacExtensionInterface(FacExtensionInterface::FacNetlistWriter)
    {
        m_description = "Default GEXF Writer";
        m_supported_file_extensions.push_back(".gexf");
        FacFactoryProvider<NetlistWriter>* fac = new FacFactoryProvider<NetlistWriter>;
        fac->m_factory = []() { return std::make_unique<GexfWriter>(); };
        factory_provider = fac;
    }

    GexfWriterPlugin::GexfWriterPlugin()
        : m_extension(nullptr)
    {;}

    std::string GexfWriterPlugin::get_name() const
    {
        return std::string("gexf_writer");
    }

    std::string GexfWriterPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void GexfWriterPlugin::on_load()
    {
        m_extension = new GexfWriterExtension;
    }

    void GexfWriterPlugin::on_unload()
    {
        delete m_extension;
    }

    std::vector<AbstractExtensionInterface*> GexfWriterPlugin::get_extensions() const
    {
        return std::vector<AbstractExtensionInterface*>({m_extension});
    }
}    // namespace hal
