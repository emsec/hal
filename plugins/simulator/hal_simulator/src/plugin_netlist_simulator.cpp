#include "netlist_simulator/plugin_netlist_simulator.h"
#include "netlist_simulator_controller/simulation_engine.h"

namespace hal
{
    std::string NetlistSimulatorPlugin::s_engine_name;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistSimulatorPlugin>();
    }

    std::string NetlistSimulatorPlugin::get_name() const
    {
        return std::string("hal_simulator");
    }

    std::string NetlistSimulatorPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string NetlistSimulatorPlugin::get_description() const
    {
        return std::string("Deprecated HAL build-in simulation engine (for tests only)");
    }

    std::set<std::string> NetlistSimulatorPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        return retval;
    }

    void NetlistSimulatorPlugin::on_load()
    {
        // constructor will register with controller
        s_engine_name = (new NetlistSimulatorFactory)->name();
    }

    void NetlistSimulatorPlugin::on_unload()
    {
        SimulationEngineFactories::instance()->deleteFactory(s_engine_name);
    }
}
