#include "netlist_simulator_controller/simulation_engine.h"

namespace hal {
    SimulationEngines* SimulationEngines::inst = nullptr;

    SimulationEngines* SimulationEngines::instance()
    {
        if (!inst) inst = new SimulationEngines;
        return inst;
    }
}
