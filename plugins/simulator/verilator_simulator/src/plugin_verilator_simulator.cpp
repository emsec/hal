#include "netlist_simulator_controller/simulation_engine.h"
#include "verilator_simulator/plugin_verilator_simulator.h"
#include "verilator_simulator/verilator_simulator.h"

namespace hal {

std::string VerilatorSimulatorPlugin::s_engine_name;

extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
{
    return std::make_unique<VerilatorSimulatorPlugin>();
}

std::string VerilatorSimulatorPlugin::get_name() const
{
    return std::string("verilator_simulator");
}

std::string VerilatorSimulatorPlugin::get_version() const
{
    return std::string("0.1");
}

void VerilatorSimulatorPlugin::on_load()
{
    // constructor will register with controller
    s_engine_name = (new verilator_simulator::VerilatorEngine)->name();
}

void VerilatorSimulatorPlugin::on_unload()
{
    SimulationEngines::instance()->deleteEngine(s_engine_name);
}

}
