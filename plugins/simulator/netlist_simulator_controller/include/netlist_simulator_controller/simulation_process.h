#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QProcess>

namespace hal {

    class NetlistSimulatorController;

    class SimulationProcess : public QObject {
        Q_OBJECT

        SimulationEngineScripted* mEngine;

        int mLineIndex;
        int mNumberLines;
        QProcess* mProcess;

        bool launchProcess();

    Q_SIGNALS:
        void processFinished(bool success);

    private Q_SLOTS:
        void handleStepFinished(int exitCode, QProcess::ExitStatus exitStatus);

    public:
        SimulationProcess(NetlistSimulatorController* controller, SimulationEngineScripted* engine);

        void start();
    };
}
