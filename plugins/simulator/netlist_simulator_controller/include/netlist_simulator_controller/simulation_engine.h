#pragma once
#include <vector>
#include <string>
#include "netlist_simulator_controller/simulation_input.h"

namespace hal {

    class SimulationEngine {
        std::string mName;
    public:
        SimulationEngine(const std::string& nam) : mName(nam) {;}
        virtual void setSimulationInput(SimulationInput* simInput) = 0;
        virtual void run() = 0;
        std::string name() const { return mName; }
    };

    class SimulationEngines : public std::vector<SimulationEngine*>
    {
        SimulationEngines() {;}
        static SimulationEngines* inst;
    public:
        static SimulationEngines* instance();
    };
}
