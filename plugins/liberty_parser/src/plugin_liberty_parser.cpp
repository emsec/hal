#include "plugin_liberty_parser.h"
#include "netlist/gate_library/gate_library_manager.h"

namespace hal
{

    extern std::unique_ptr<BasePluginInterface> get_plugin_instance()
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
        gate_library_manager::register_parser(&m_parser, {".lib"});
    }

    void LibertyParserPlugin::on_unload()
    {
        gate_library_manager::unregister_parser(&m_parser);
    }
}
