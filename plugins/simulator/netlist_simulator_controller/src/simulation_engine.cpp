#include "netlist_simulator_controller/simulation_engine.h"

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_process.h"
#include "netlist_simulator_controller/simulation_thread.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryDir>
#include <QThread>

namespace hal
{
    SimulationEngine::SimulationEngine(const std::string& nam)
        : mName(nam), mRequireClockEvents(false), mCanShareMemory(false), mState(Preparing), mSimulationInput(nullptr)
    {
        QString templatePath = QDir::tempPath();
        if (!templatePath.isEmpty())
            templatePath += '/';
        templatePath += "hal_simulation_" + QString::fromStdString(mName) + "_XXXXXX";
        mTempDir = new QTemporaryDir(templatePath);
    }

    SimulationEngine::~SimulationEngine()
    {
        delete mTempDir;
    }

    std::string SimulationEngine::directory() const
    {
        return mTempDir->path().toStdString();
    }

    bool SimulationEngine::finalize()
    {
        mState = Done;
        return true;
    }

    void SimulationEngine::failed()
    {
        mState = Failed;
    }

    void SimulationEngine::set_engine_property(const std::string& key, const std::string& value)
    {
        mProperties[key] = value;
    }

    std::string SimulationEngine::get_engine_property(const std::string& key)
    {
        if (mProperties.find(key) != mProperties.end())
        {
            return mProperties[key];
        }

        return std::string();
    }

    SimulationEngineEventDriven::SimulationEngineEventDriven(const std::string& nam) : SimulationEngine(nam)
    {
        mCanShareMemory = true;
    }

    std::vector<WaveEvent> SimulationEngineEventDriven::get_simulation_events(u32 netId) const
    {
        Q_UNUSED(netId);
        return std::vector<WaveEvent>();
    }

    bool SimulationEngineEventDriven::setSimulationInput(SimulationInput* simInput)
    {
        mSimulationInput = simInput;
        return true;
    }

    bool SimulationEngineEventDriven::run(NetlistSimulatorController* controller)
    {
        SimulationThread* thread = new SimulationThread(controller, mSimulationInput, this);
        mState                   = Running;
        thread->start();
        return true;
    }

    bool SimulationEngineScripted::run(NetlistSimulatorController* controller)
    {
        SimulationProcess* proc = new SimulationProcess(controller, this);
        mState                  = Running;
        proc->start();
        return true;
    }

    //======================= FACTORY ================================
    SimulationEngineFactory::SimulationEngineFactory(const std::string& nam) : mName(nam)
    {
        SimulationEngineFactories::instance()->push_back(this);
    }

    SimulationEngineFactories* SimulationEngineFactories::inst = nullptr;

    SimulationEngineFactories* SimulationEngineFactories::instance()
    {
        if (!inst)
            inst = new SimulationEngineFactories;
        return inst;
    }

    std::vector<std::string> SimulationEngineFactories::factoryNames() const
    {
        std::vector<std::string> retval;
        for (auto it = begin(); it != end(); ++it)
            retval.push_back((*it)->name());
        return retval;
    }

    SimulationEngineFactory* SimulationEngineFactories::factoryByName(const std::string nam) const
    {
        for (auto it = begin(); it != end(); ++it)
            if ((*it)->name() == nam)
                return (*it);

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

}    // namespace hal
