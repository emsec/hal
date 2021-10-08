#pragma once

#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include <QProcess>

namespace hal {

    class SimulationProcess : public QObject {
        Q_OBJECT

        SimulationEngineScripted* mEngine;

        int mLineIndex;
        int mNumberLines;
        QProcess* mProcess;

        bool launchProcess();
    private Q_SLOTS:
        void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    public:
        SimulationProcess(SimulationEngineScripted* engine);

        void start();
    };
}
