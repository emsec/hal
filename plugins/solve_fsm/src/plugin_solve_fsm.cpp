#include "solve_fsm/plugin_solve_fsm.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<SolveFsmPlugin>();
    }

    std::string SolveFsmPlugin::get_name() const
    {
        return std::string("solve_fsm");
    }

    std::string SolveFsmPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string SolveFsmPlugin::get_description() const
    {
        return "Plugin to automatically generate FSM state transition graphs for given FSMs.";
    }

}    // namespace hal