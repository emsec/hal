#include "netlist_simulator_controller/simulation_engine.h"

namespace hal {
    SimulationEngines* SimulationEngines::inst = nullptr;

    SimulationEngines* SimulationEngines::instance()
    {
        if (!inst) inst = new SimulationEngines;
        return inst;
    }

    SimulationEngine::SimulationEngine(const std::string& nam) : mName(nam)
    {
        SimulationEngines::instance()->push_back(this);
    }


}
