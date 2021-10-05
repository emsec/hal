#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QThread>

namespace hal {

    class SimulationThread : public QThread {
        Q_OBJECT

        const SimulationInput* mSimulationInput;
        SimulationEngineEventDriven* mEngine;
    public:
        SimulationThread(const SimulationInput* simInput, SimulationEngineEventDriven* engine);

        void run() override;
    };
}
