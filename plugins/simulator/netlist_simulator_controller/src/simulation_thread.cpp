#include "netlist_simulator_controller/simulation_thread.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include <QProcess>
#include <QThread>
#include <vector>

namespace hal {

    SimulationThread::SimulationThread(NetlistSimulatorController* controller, const SimulationInput* simInput, SimulationEngineEventDriven *engine)
        : QThread(controller), mSimulationInput(simInput), mEngine(engine)
    {
        connect(this, &SimulationThread::threadFinished, controller, &NetlistSimulatorController::handleRunFinished);
    }


    void SimulationThread::run()
    {
        for (const SimulationInputNetEvent& simEvt : mSimulationInput->get_simulation_net_events())
            mEngine->inputEvent(simEvt);
        Q_EMIT threadFinished(true);
    }
}
