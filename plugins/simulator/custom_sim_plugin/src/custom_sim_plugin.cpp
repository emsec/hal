#include "custom_sim_plugin/custom_sim_plugin.h"
#include "custom_sim_plugin/custom_sim_engine.h"
#include "netlist_simulator_controller/simulation_engine.h"

namespace hal
{
    std::string CustomSimPlugin::s_engine_name;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<CustomSimPlugin>();
    }

    std::string CustomSimPlugin::get_name() const
    {
        return std::string("custom_sim_plugin");
    }

    std::string CustomSimPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string CustomSimPlugin::get_description() const
    {
        return std::string("Custom Sim Plugin");
    }

    std::set<std::string> CustomSimPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        return retval;
    }

    void CustomSimPlugin::on_load()
    {
        // constructor will register with controller
        s_engine_name = (new CustomSimEngineFactory)->name();
    }

    void CustomSimPlugin::on_unload()
    {
        SimulationEngineFactories::instance()->deleteFactory(s_engine_name);
    }
}
