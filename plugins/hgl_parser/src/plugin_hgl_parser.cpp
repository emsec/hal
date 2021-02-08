#include "hgl_parser/plugin_hgl_parser.h"

#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "hgl_parser/hgl_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<HGLParserPlugin>();
    }

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
        gate_library_parser_manager::register_parser("Default HGL Parser", []() { return std::make_unique<HGLParser>(); }, {".hgl"});
    }

    void HGLParserPlugin::on_unload()
    {
        gate_library_parser_manager::unregister_parser("Default HGL Parser");
    }
}    // namespace hal
