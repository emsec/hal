#include "netlist_simulator_controller/simulation_thread.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "hal_core/utilities/log.h"
#include <QProcess>
#include <QThread>
#include <vector>

namespace hal {

    SimulationThread::SimulationThread(NetlistSimulatorController* controller, const SimulationInput* simInput, SimulationEngineEventDriven *engine)
        : QThread(controller), mSimulationInput(simInput), mEngine(engine), mLogChannel(controller->get_name())
    {
        connect(this, &SimulationThread::threadFinished, controller, &NetlistSimulatorController::handleRunFinished);
    }

    void SimulationThread::terminateThread(bool success, const char* failedStep)
    {
        if (!success)
        {
            mEngine->failed();
            if (failedStep)
               log_warning(mLogChannel, "simulation engine error during {}.", failedStep);
        }
        Q_EMIT threadFinished(success);
    }

    void SimulationThread::run()
    {
        for (const SimulationInputNetEvent& simEvt : mSimulationInput->get_simulation_net_events())
            if (!mEngine->inputEvent(simEvt))
                return terminateThread(false, "run");

        terminateThread(mEngine->finalize(), "finalize");
    }
}
