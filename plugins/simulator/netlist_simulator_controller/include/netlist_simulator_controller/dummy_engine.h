#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_event.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/defines.h"

namespace hal {

    class DummyEngine : public SimulationEngineScripted
    {
    public:
        DummyEngine(const std::string& nam) : SimulationEngineScripted(nam) {;}

        virtual bool setSimulationInput(SimulationInput* simInput);
        int numberCommandLines() const;
        std::vector<std::string> commandLine(int lineIndex) const;
        bool finalize();
    };

    class DummyEngineFactory : SimulationEngineFactory
    {
        static DummyEngineFactory* sDumFac;
    public:
        DummyEngineFactory() : SimulationEngineFactory("dummy_engine") {;}
        SimulationEngine* createEngine() const override { return new DummyEngine(mName); }
    };
}
