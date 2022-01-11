#include "verilator/plugin_verilator.h"

#include "hal_core/netlist/netlist.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include "verilator/verilator.h"

namespace hal
{
    std::string VerilatorSimulatorPlugin::s_engine_name;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<VerilatorSimulatorPlugin>();
    }

    std::string VerilatorSimulatorPlugin::get_name() const
    {
        return std::string("verilator");
    }

    std::string VerilatorSimulatorPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void VerilatorSimulatorPlugin::on_load()
    {
        // constructor will register with controller
        s_engine_name = (new verilator::VerilatorEngineFactory)->name();
    }

    void VerilatorSimulatorPlugin::on_unload()
    {
        SimulationEngineFactories::instance()->deleteFactory(s_engine_name);
    }
}    // namespace hal
