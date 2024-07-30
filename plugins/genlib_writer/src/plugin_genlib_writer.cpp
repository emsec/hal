#include "genlib_writer/plugin_genlib_writer.h"

#include "genlib_writer/genlib_writer.h"
#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<GenlibWriterPlugin>();
    }

    std::string GenlibWriterPlugin::get_name() const
    {
        return std::string("genlib_writer");
    }

    std::string GenlibWriterPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void GenlibWriterPlugin::on_load()
    {
        gate_library_writer_manager::register_writer("Default Genlib Writer", []() { return std::make_unique<GenlibWriter>(); }, {".genlib"});
    }

    void GenlibWriterPlugin::on_unload()
    {
        gate_library_writer_manager::unregister_writer("Default Genlib Writer");
    }

}    // namespace hal
