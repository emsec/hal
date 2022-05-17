#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QThread>
#include <string>

namespace hal {

    class NetlistSimulatorController;

    class SimulationThread : public QThread {
        Q_OBJECT

        const SimulationInput* mSimulationInput;
        SimulationEngineEventDriven* mEngine;
        std::string mLogChannel;

        u64 mSimulTime;
        SimulationInputNetEvent mSimulationInputNetEvent;
        std::string mSaleaeDirectoryFilename;

        void terminateThread(bool success, const char* failedStep = nullptr);
    Q_SIGNALS:
        void threadFinished(bool success);

    public:
        SimulationThread(NetlistSimulatorController* controller, const SimulationInput* simInput, SimulationEngineEventDriven* engine);

        void run() override;
    };
}
