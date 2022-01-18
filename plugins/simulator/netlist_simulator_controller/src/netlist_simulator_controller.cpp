#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/dummy_engine.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_parser.h"

#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/vcd_serializer.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_version.h"

#include <QFile>
#include <QDate>
#include <QVector>
#include <QDebug>
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/netlist_utils.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"

namespace hal
{
    NetlistSimulatorController::NetlistSimulatorController(u32 id, const std::string nam, QObject *parent)
        : QObject(parent), mId(id), mName(QString::fromStdString(nam)), mState(NoGatesSelected), mSimulationEngine(nullptr),
          mWaveDataList(new WaveDataList), mSimulationInput(new SimulationInput)
    {
        if (mName.isEmpty()) mName = QString("sim_controller%1").arg(mId);
        LogManager::get_instance().add_channel(mName.toStdString(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        NetlistSimulatorControllerMap::instance()->addController(this);
    }

    NetlistSimulatorController::~NetlistSimulatorController()
    {
        NetlistSimulatorControllerMap::instance()->removeController(mId);
        mWaveDataList->deleteLater();
        delete mSimulationInput;
    }

    void NetlistSimulatorController::setState(SimulationState stat)
    {
        if (stat==mState) return;
        mState = stat;
        switch (mState)
        {
        case NoGatesSelected:  log_info(get_name(), "Select gates for simulation");              break;
        case ParameterSetup:   log_info(get_name(), "Expecting parameter and input");            break;
        case ParameterReady:   log_info(get_name(), "Preconditions to start simulation met");    break;
        case SimulationRun:    log_info(get_name(), "Running simulation, please wait...");       break;
        case ShowResults:      log_info(get_name(), "Simulation engine completed successfully"); break;
        case EngineFailed:
            log_info(get_name(), "Simulation engine process error");
            if (mSimulationEngine) mSimulationEngine->failed();
            break;
        }
        Q_EMIT stateChanged(mState);
    }

    SimulationEngine* NetlistSimulatorController::create_simulation_engine(const std::string& name)
    {
        SimulationEngineFactory* fac = SimulationEngineFactories::instance()->factoryByName(name);
        if (!fac) return nullptr;
        if (mSimulationEngine) delete mSimulationEngine;
        mSimulationEngine = fac->createEngine();
        log_info(get_name(), "engine '{}' created with working directory {}.", mSimulationEngine->name(), mSimulationEngine->directory());
        checkReadyState();
        return mSimulationEngine;
    }

    SimulationEngine* NetlistSimulatorController::get_simulation_engine() const
    {
        return mSimulationEngine;
    }

    std::vector<std::string> NetlistSimulatorController::get_engine_names() const
    {
        return SimulationEngineFactories::instance()->factoryNames();
    }

    void NetlistSimulatorController::initSimulator()
    {
    }

    void NetlistSimulatorController::set_no_clock_used()
    {
        mSimulationInput->set_no_clock_used();
        checkReadyState();
    }

    u32 NetlistSimulatorController::add_waveform_group(const std::string& name, const std::vector<Net*> nets)
    {
        if (name.empty()) return 0;
        QVector<u32> netVector;
        netVector.reserve(nets.size());
        for (Net* n : nets) netVector.append(n->get_id());
        u32 grpId = mWaveDataList->createGroup(QString::fromStdString(name));
        mWaveDataList->addNetsToGroup(grpId, netVector);
        return grpId;
    }

    void NetlistSimulatorController::remove_waveform_group(u32 group_id)
    {
        mWaveDataList->removeGroup(group_id);
    }

    void NetlistSimulatorController::handleOpenInputFile(const QString &filename)
    {
        if (filename.isEmpty()) return;
        if (!mSimulationEngine)
        {
            log_warning(get_name(), "No engine selected, cannot import simulation data.");
            return;
        }
        VcdSerializer reader;
        QList<const Net*> onlyNets;
        for (const Net* n : mSimulationInput->get_input_nets()) onlyNets.append(n);
        if (reader.importVcd(filename,QString::fromStdString(mSimulationEngine->directory()),onlyNets))
        {
            SaleaeDirectory sd(mSimulationEngine->get_saleae_directory_filename());
            for (const Net* n : onlyNets)
            {
                WaveData* wd = new WaveData(n);
                wd->loadSaleae(sd);
                mWaveDataList->addOrReplace(wd);
            }
        }
        checkReadyState();
    }

    bool NetlistSimulatorController::run_simulation()
    {
        if (!mSimulationEngine)
        {
            log_warning(get_name(), "no simulation engine selected");
            return false;
        }

        if (mState != ParameterReady)
        {
            log_warning(get_name(), "wrong state {}.", (u32) mState);
            return false;
        }

        mWaveDataList->setValueForEmpty(0);

        for (auto it = mBadAssignInputWarnings.constBegin(); it != mBadAssignInputWarnings.constEnd(); ++it)
            if (it.value() > 3)
                log_warning(get_name(), "Totally {} attempts to set input values for net ID={}, but net is not an input.", it.value(), it.key());

        struct WaveIterator
        {
            const Net* n;
            const WaveData* wd;
            QMap<u64,int>::const_iterator it;
        };

        QMultiMap<u64,WaveIterator> nextInTimeLine;

        // generate clock events if required
        if (mSimulationEngine->clock_events_required())
        {
            for (const Net* n : mSimulationInput->get_input_nets())
            {
                if (!mSimulationInput->is_clock(n)) continue;
                SimulationInput::Clock clk;
                for (const SimulationInput::Clock& testClk : mSimulationInput->get_clocks())
                {
                    if (testClk.clock_net == n)
                    {
                        clk = testClk;
                        break;
                    }
                }
                WaveDataClock* wdc = new WaveDataClock(n, clk, mWaveDataList->simulTime());
                mWaveDataList->addOrReplace(wdc);
                SaleaeDirectory sd(mSimulationEngine->get_saleae_directory_filename());
                wdc->saveSaleae(sd);
            }
        }

        if (!mSimulationEngine->setSimulationInput(mSimulationInput))
        {
            log_warning(get_name(), "simulation engine error during setup.");
            setState(EngineFailed);
            return false;
        }

        if (!mSimulationEngine->run(this))
        {
            log_warning(get_name(), "simulation engine error during startup.");
            setState(EngineFailed);
            return false;
        }
        setState(SimulationRun);
        return true;
    }

    WaveData* NetlistSimulatorController::get_waveform_by_net(Net* n) const
    {
        WaveData* wd = mWaveDataList->waveDataByNetId(n->get_id());
        if (wd) return wd;
        if (!mSimulationEngine) return nullptr;
        SaleaeDirectory sd(mSimulationEngine->get_saleae_directory_filename());
        if (!wd->loadSaleae(sd)) return nullptr;
        return wd;
    }

    std::vector<const Net *> NetlistSimulatorController::getFilterNets(FilterInputFlag filter) const
    {
        if (!mSimulationInput->has_gates()) return std::vector<const Net*>();
        switch (filter)
        {
        case GlobalInputs:
            return std::vector<const Net*>(get_input_nets().begin(), get_input_nets().end());
        case PartialNetlist:
            return get_partial_netlist_nets();
        case CompleteNetlist:
        {
            std::vector<Net*> tmp = (*mSimulationInput->get_gates().begin())->get_netlist()->get_nets();
            return  std::vector<const Net*>(tmp.begin(),tmp.end());
        }
        case NoFilter:
            break;
        }
        return std::vector<const Net*>();
    }

    bool NetlistSimulatorController::import_vcd(const std::string& filename, FilterInputFlag filter)
    {
        if (!mSimulationEngine)
        {
            log_warning(get_name(), "No engine selected, cannot import simulation data.");
            return false;
        }

        VcdSerializer reader;

        QList<const Net*> inputNets;
        if (filter != NoFilter)
            for (const Net* n: getFilterNets(filter)) inputNets.append(n);

        if (reader.importVcd(QString::fromStdString(filename),QString::fromStdString(mSimulationEngine->directory()),inputNets))
        {
            SaleaeDirectory sd(mSimulationEngine->get_saleae_directory_filename());
            mWaveDataList->updateFromSaleae(sd);
        }
        else
        {
            return false;
        }
        
        checkReadyState();
        Q_EMIT parseComplete();
        return true;
    }

    void NetlistSimulatorController::import_csv(const std::string& filename, FilterInputFlag filter, u64 timescale)
    {
        if (!mSimulationEngine)
        {
            log_warning(get_name(), "No engine selected, cannot import simulation data.");
            return;
        }
        VcdSerializer reader;

        QList<const Net*> inputNets;
        if (filter != NoFilter)
            for (const Net* n: getFilterNets(filter)) inputNets.append(n);

        if (reader.importCsv(QString::fromStdString(filename),QString::fromStdString(mSimulationEngine->directory()),inputNets,timescale))
        {
            SaleaeDirectory sd(mSimulationEngine->get_saleae_directory_filename());
            mWaveDataList->updateFromSaleae(sd);
        }
        checkReadyState();
        Q_EMIT parseComplete();
    }

    void NetlistSimulatorController::set_saleae_timescale(u64 timescale)
    {
        SaleaeParser::sTimeScaleFactor = timescale;
    }

    void NetlistSimulatorController::handleRunFinished(bool success)
    {
        if (!success)
        {
            log_warning(get_name(), "simulation engine error during run.");
            setState(EngineFailed);
        }

        Q_EMIT engineFinished(success);
        /*
       for (Net* n : gNetlist->get_nets())
        {
            WaveData* wd = WaveData::simulationResultFactory(n, mSimulator.get());
            if (wd) mResultMap.insert(wd->id(),wd);
        }

        mSimulator->generate_vcd("result.vcd",0,t);

*/
    }

    bool NetlistSimulatorController::get_results()
    {
        bool success = getResultsInternal();
        setState(success ? ShowResults : EngineFailed);
        return success;
    }

    bool NetlistSimulatorController::getResultsInternal()
    {
        SimulationEngineEventDriven* sevd = static_cast<SimulationEngineEventDriven*>(mSimulationEngine);
        // mWaveDataList->dump();
        if (mSimulationEngine->can_share_memory())
        {
            for (const Net* n : get_partial_netlist_nets())
            {
                WaveData* wd = new WaveData(n);
                for (WaveEvent evt : sevd->get_simulation_events(n->get_id()))
                {
                    wd->insertBooleanValue(evt.time,evt.new_value);
                }
       //         if (wd->isEmpty()) continue;
                mWaveDataList->addOrReplace(wd);
            }
        }
        else
        {
            std::filesystem::path resultFile = mSimulationEngine->get_result_filename();
            if (resultFile.is_relative())
                resultFile = mSimulationEngine->directory() / resultFile;
            VcdSerializer reader(this);
            QFileInfo info(QString::fromStdString(resultFile.string()));
            if (!info.exists() || !info.isReadable()) return false;

            QList<const Net*> partialNets;
            for (const Net* n : get_partial_netlist_nets())
                partialNets.append(n);

            if (!reader.importVcd(QString::fromStdString(resultFile),QString::fromStdString(mSimulationEngine->directory()),partialNets)) return false;
        }
        return true;
    }

    void NetlistSimulatorController::add_clock_frequency(const Net* clock_net, u64 frequency, bool start_at_zero)
    {
        u64 period = 1'000'000'000'000ul / frequency;
        add_clock_period(clock_net, period, start_at_zero);
    }

    void NetlistSimulatorController::checkReadyState()
    {
        if (mState >= ParameterReady) return; // nothing to do

        if (mSimulationInput->is_ready() && mSimulationEngine && mWaveDataList->simulTime() > 0)
            setState(ParameterReady);
    }

    void NetlistSimulatorController::add_clock_period(const Net* clock_net, u64 period, bool start_at_zero, u64 duration)
    {
        SimulationInput::Clock clk;
        clk.clock_net     = clock_net;
        clk.switch_time   = period / 2;
        clk.start_at_zero = start_at_zero;
        mSimulationInput->add_clock(clk);
        WaveData* wd = new WaveDataClock(clock_net, clk, duration ? duration : 2000);
        mWaveDataList->addOrReplace(wd);
        checkReadyState();
    }

    void NetlistSimulatorController::add_gates(const std::vector<Gate *> &gates)
    {
        mSimulationInput->add_gates(gates);

        QSet<u32> previousInputSet = mWaveDataList->toSet();
        QSet<u32> currentInputSet;
        for (const Net* n : mSimulationInput->get_input_nets())
        {
            u32 nid = n->get_id();
            if (!previousInputSet.contains(nid))
            {
                WaveData* wd = new WaveData(n, WaveData::InputNet);
                mWaveDataList->addOrReplace(wd);
            }
            currentInputSet.insert(nid);
        }
        previousInputSet -= currentInputSet;
        for (u32 id : previousInputSet)
        {
            mWaveDataList->remove(id);
        }
        if (mState == NoGatesSelected && mSimulationInput->has_gates()) setState(ParameterSetup);
        checkReadyState();
    }

    const std::unordered_set<const Gate*>& NetlistSimulatorController::get_gates() const
    {
        return mSimulationInput->get_gates();
    }

    const std::unordered_set<const Net*>& NetlistSimulatorController::get_input_nets() const
    {
        return mSimulationInput->get_input_nets();
    }

    const std::vector<const Net*>& NetlistSimulatorController::get_output_nets() const
    {
        return mSimulationInput->get_output_nets();
    }

    const std::vector<const Net*>& NetlistSimulatorController::get_partial_netlist_nets() const
    {
        return mSimulationInput->get_partial_netlist_nets();
    }

    void NetlistSimulatorController::set_input(const Net* net, BooleanFunction::Value value)
    {
        Q_ASSERT(net);
        if (!mSimulationEngine)
        {
            log_warning(get_name(), "no engine selected, no input value assigned.");
            return;
        }
        if (!mSimulationInput->is_input_net(net))
        {
            if (mBadAssignInputWarnings[net->get_id()]++ < 3)
                log_warning(get_name(), "net[{}] '{}' is not an input net, value not assigned.", net->get_id(), net->get_name());
            return;
        }
        WaveData* wd = mWaveDataList->waveDataByNetId(net->get_id());
        if (!wd)
        {
            wd = new WaveData(net);
            mWaveDataList->add(wd,false);
        }
        u64 t = mWaveDataList->simulTime();
        wd->insertBooleanValue(t,value);
        SaleaeDirectory sd(mSimulationEngine->get_saleae_directory_filename());
        wd->saveSaleae(sd);
    }

    void NetlistSimulatorController::initialize()
    {
    }

    void NetlistSimulatorController::reset()
    {
        mWaveDataList->clearAll();
    }

    void NetlistSimulatorController::simulate(u64 picoseconds)
    {
       mWaveDataList->incrementSimulTime(picoseconds);
       checkReadyState();
    }

    void NetlistSimulatorController::handleSelectGates()
    {

        /*
        mSimulateGates = gsd.selectedGates();
        initSimulator();
        */
    }

    bool NetlistSimulatorController::generate_vcd(const std::filesystem::path& path, u32 start_time, u32 end_time, std::set<const Net*> nets) const
    {
        VcdSerializer writer;
        QList<const WaveData*> partialList = mWaveDataList->partialList(start_time, end_time, nets);
        bool success = writer.serialize(QString::fromStdString(path.string()), partialList);
        for (const WaveData* wd : partialList)
            delete wd;
        return success;
    }

    NetlistSimulatorControllerMap* NetlistSimulatorControllerMap::sInst = nullptr;

    NetlistSimulatorControllerMap* NetlistSimulatorControllerMap::instance()
    {
        if (!sInst) sInst = new NetlistSimulatorControllerMap;
        return sInst;
    }

    void NetlistSimulatorControllerMap::addController(NetlistSimulatorController* ctrl)
    {
        u32 id = ctrl->get_id();
        mMap.insert(id,ctrl);
        Q_EMIT controllerAdded(id);
    }

    void NetlistSimulatorControllerMap::removeController(u32 id)
    {
        auto it = mMap.find(id);
        if (it == mMap.end()) return;
        mMap.erase(it);
        Q_EMIT controllerRemoved(id);
    }

    /* show results
    void NetlistSimulatorController::handleSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);
        for (u32 nid : gSelectionRelay->selectedNetsList())
        {
            Net* n = gNetlist->get_net_by_id(nid);
            if (!n) continue;
            const WaveData* wd = mResultMap.value(n->get_id());
            if (!wd) continue;
            WaveData* wdCopy = new WaveData(*wd);
  //          mWaveDataList->addOrReplace(wdCopy);
        }
    }
        */
}    // namespace hal
