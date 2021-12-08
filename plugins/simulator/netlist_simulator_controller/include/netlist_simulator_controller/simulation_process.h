#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QProcess>
#include <QThread>

namespace hal {

    class NetlistSimulatorController;

    class SimulationProcess : public QThread {
        Q_OBJECT

        SimulationEngineScripted* mEngine;

        int mLineIndex;
        int mNumberLines;
        QProcess* mProcess;

        void abortOnError();

    Q_SIGNALS:
        void processFinished(bool success);

    public:
        SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine);

        void run();
    private:
        void runLocal();

        // server execution
        void runRemote();

    };
}
