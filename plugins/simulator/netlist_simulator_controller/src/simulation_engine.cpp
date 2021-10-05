#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_thread.h"
#include "netlist_simulator_controller/simulation_process.h"
#include <QProcess>
#include <QThread>

namespace hal {
    SimulationEngines* SimulationEngines::inst = nullptr;

    SimulationEngines* SimulationEngines::instance()
    {
        if (!inst) inst = new SimulationEngines;
        return inst;
    }

    std::vector<std::string> SimulationEngines::names() const
    {
        std::vector<std::string> retval;
        for (auto it=begin(); it!=end(); ++it)
            retval.push_back((*it)->name());
        return retval;
    }

    SimulationEngine* SimulationEngines::engineByName(const std::string nam) const
    {
        for (auto it=begin(); it!=end(); ++it)
            if ((*it)->name() == nam) return (*it);

        return nullptr;
    }

    void SimulationEngines::deleteEngine(const std::string nam)
    {
        auto it = begin();
        while (it != end())
        {
            if ((*it)->name() == nam)
            {
                delete (*it);
                it = erase(it);
            }
            else
                ++it;
        }
    }

    SimulationEngine::SimulationEngine(const std::string& nam) : mName(nam)
    {
        SimulationEngines::instance()->push_back(this);
    }


    std::vector<WaveEvent> SimulationEngineEventDriven::get_simulation_events(Net *n) const
    {
        Q_UNUSED(n);
        return std::vector<WaveEvent>();
    }

    bool SimulationEngineEventDriven::setSimulationInput(SimulationInput *simInput)
    {
        mSimulationInput = simInput;
        return true;
    }

    bool SimulationEngineEventDriven::run()
    {
        SimulationThread* thread = new SimulationThread(mSimulationInput,this);
        thread->start();
        return true;
    }

    bool SimulationEngineScripted::run()
    {
        SimulationProcess* proc = new SimulationProcess(this);
        proc->start();
        return true;
    }
}
