#include "netlist_simulator_controller/simulation_thread.h"
#include <QProcess>
#include <QThread>
#include <vector>

namespace hal {

    SimulationThread::SimulationThread(const SimulationInput* simInput, SimulationEngineEventDriven *engine)
        : mSimulationInput(simInput), mEngine(engine) {;}


    void SimulationThread::run()
    {
        for (const SimulationInputNetEvent& simEvt : mSimulationInput->get_simulation_net_events())
            mEngine->inputEvent(simEvt);
    }
}
