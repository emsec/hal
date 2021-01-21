#include "liberty_parser/plugin_liberty_parser.h"

#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "liberty_parser/liberty_parser.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<LibertyParserPlugin>();
    }

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
        gate_library_parser_manager::register_parser("Default Liberty Parser", []() { return std::make_unique<LibertyParser>(); }, {".lib"});
    }

    void LibertyParserPlugin::on_unload()
    {
        gate_library_parser_manager::unregister_parser("Default Liberty Parser");
    }
}    // namespace hal
