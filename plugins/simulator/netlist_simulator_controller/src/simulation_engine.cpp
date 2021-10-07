#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_thread.h"
#include "netlist_simulator_controller/simulation_process.h"
#include <QProcess>
#include <QThread>
#include <QTemporaryDir>

namespace hal {
    SimulationEngine::SimulationEngine(const std::string& nam)
        : mName(nam), mRequireClockEvents(false), mCanShareMemory(false), mState(Preparing), mTempDir(new QTemporaryDir)
    {;}

    SimulationEngine::~SimulationEngine()
    {
        delete mTempDir;
    }

    std::string SimulationEngine::directory() const
    {
        return mTempDir->path().toStdString();
    }

    SimulationEngineEventDriven::SimulationEngineEventDriven(const std::string& nam)
        : SimulationEngine(nam), mSimulationInput(nullptr)
    {
        mCanShareMemory = true;
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
        mState = Running;
        thread->start();
        return true;
    }

    bool SimulationEngineScripted::run()
    {
        SimulationProcess* proc = new SimulationProcess(this);
        mState = Running;
        proc->start();
        return true;
    }


    //======================= FACTORY ================================
    SimulationEngineFactory::SimulationEngineFactory(const std::string& nam)
        : mName(nam)
    {
        SimulationEngineFactories::instance()->push_back(this);
    }

    SimulationEngineFactories* SimulationEngineFactories::inst = nullptr;

    SimulationEngineFactories* SimulationEngineFactories::instance()
    {
        if (!inst) inst = new SimulationEngineFactories;
        return inst;
    }

    std::vector<std::string> SimulationEngineFactories::factoryNames() const
    {
        std::vector<std::string> retval;
        for (auto it=begin(); it!=end(); ++it)
            retval.push_back((*it)->name());
        return retval;
    }

    SimulationEngineFactory* SimulationEngineFactories::factoryByName(const std::string nam) const
    {
        for (auto it=begin(); it!=end(); ++it)
            if ((*it)->name() == nam) return (*it);

        return nullptr;
    }

    void SimulationEngineFactories::deleteFactory(const std::string nam)
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


}
