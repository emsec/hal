#include "netlist_simulator_controller/simulation_engine.h"

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_process.h"
#include "netlist_simulator_controller/simulation_thread.h"

#include <QDir>
#include <QProcess>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace hal
{
    SimulationEngine::SimulationEngine(const std::string& nam)
        : mName(nam), mRequireClockEvents(false), mCanShareMemory(false), mState(Preparing),
          mSimulationInput(nullptr)
    {;}

    std::string SimulationEngine::get_working_directory() const
    {
        return mWorkDir;
    }

    void SimulationEngine::set_working_directory(const std::string& workDir)
    {
        mWorkDir = workDir;
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

    bool SimulationEngine::install_saleae_parser(std::string dirname) const
    {
        QDir dir(QString::fromStdString(dirname));
        if (!dir.exists()) return false;
        const char* filenames[] = {":/include/saleae_parser.h", ":/src/saleae_parser.cpp",
                                   ":/include/saleae_file.h", ":/src/saleae_file.cpp",
                                   ":/include/saleae_directory.h", ":/src/saleae_directory.cpp", nullptr};
        for (int i=0; filenames[i]; i++)
        {
            // add STAMDALONE_PARSER preprocessor directive to all source files
            QFileInfo finfo(filenames[i]);
            QString targetFile = dir.absoluteFilePath(finfo.fileName());
            QFile ff(filenames[i]);
            if (!ff.open(QIODevice::ReadOnly)) return false;
            QFile of(targetFile);
            if (!of.open(QIODevice::WriteOnly)) return false;
            of.write("#define STANDALONE_PARSER 1\n");
            of.write(ff.readAll());
        }
        return true;
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

    bool SimulationEngineEventDriven::run(NetlistSimulatorController* controller, SimulationLogReceiver *logReceiver)
    {
        Q_UNUSED(logReceiver);
        SimulationThread* thread = new SimulationThread(controller, mSimulationInput, this);
        mState                   = Running;
        thread->start();
        return true;
    }

    bool SimulationEngineScripted::run(NetlistSimulatorController* controller, SimulationLogReceiver *logReceiver)
    {
        SimulationProcess* proc = new SimulationProcess(controller, this);
        proc->log()->setLogReceiver(logReceiver);
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
