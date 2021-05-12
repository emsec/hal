#include "gexf_writer/plugin_gexf_writer.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "gexf_writer/gexf_writer.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<GexfWriterPlugin>();
    }

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
        netlist_writer_manager::register_writer("Default GEXF Writer", []() { return std::make_unique<GexfWriter>(); }, {".gexf"});
    }

    void GexfWriterPlugin::on_unload()
    {
        netlist_writer_manager::unregister_writer("Default GEXF Writer");
    }
}    // namespace hal
