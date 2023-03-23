#include "verilog_writer/plugin_verilog_writer.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "verilog_writer/verilog_writer.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VerilogWriterPlugin>();
    }

    VerilogWriterExtension::VerilogWriterExtension()
        : FacExtensionInterface(FacExtensionInterface::FacNetlistWriter)
    {
        m_description = "Default Verilog Writer";
        m_supported_file_extensions.push_back(".v");
        FacFactoryProvider<NetlistWriter>* fac = new FacFactoryProvider<NetlistWriter>;
        fac->m_factory = []() { return std::make_unique<VerilogWriter>(); };
        factory_provider = fac;
    }

    VerilogWriterPlugin::VerilogWriterPlugin()
        : m_extension(nullptr)
    {;}

    std::string VerilogWriterPlugin::get_name() const
    {
        return std::string("verilog_writer");
    }

    std::string VerilogWriterPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VerilogWriterPlugin::on_load()
    {
        m_extension = new VerilogWriterExtension;
        m_extensions.push_back(m_extension);
    }

    void VerilogWriterPlugin::on_unload()
    {
        delete_extension(m_extension);
    }
}    // namespace hal
