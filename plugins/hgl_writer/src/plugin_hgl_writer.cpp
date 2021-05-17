#include "hgl_writer/plugin_hgl_writer.h"

#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"
#include "hgl_writer/hgl_writer.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<HGLWriterPlugin>();
    }

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
        gate_library_writer_manager::register_writer("Default HGL Writer", []() { return std::make_unique<HGLWriter>(); }, {".hgl"});
    }

    void HGLWriterPlugin::on_unload()
    {
        gate_library_writer_manager::unregister_writer("Default HGL Writer");
    }
}    // namespace hal
