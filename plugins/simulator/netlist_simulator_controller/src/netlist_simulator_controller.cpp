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
#include <QTemporaryDir>
#include <QDebug>
#include <QDir>
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/netlist_utils.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"

namespace hal
{
    NetlistSimulatorController::NetlistSimulatorController(u32 id, const std::string nam, QObject *parent)
        : QObject(parent), mId(id), mName(QString::fromStdString(nam)), mState(NoGatesSelected), mSimulationEngine(nullptr),
          mSimulationInput(new SimulationInput)
    {
        if (mName.isEmpty()) mName = QString("sim_controller%1").arg(mId);
        LogManager::get_instance().add_channel(mName.toStdString(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");

        QString templatePath = QDir::tempPath();
        if (!templatePath.isEmpty())
            templatePath += '/';
        templatePath += "hal_simulation_" + mName + "_XXXXXX";
        mTempDir = new QTemporaryDir(templatePath);
        QDir saleaeDir(QDir(mTempDir->path()).absoluteFilePath("saleae"));
        saleaeDir.mkpath(saleaeDir.absolutePath());
        mWaveDataList = new WaveDataList(saleaeDir.absoluteFilePath("saleae.json"));

        NetlistSimulatorControllerMap::instance()->addController(this);
    }

    NetlistSimulatorController::~NetlistSimulatorController()
    {
        NetlistSimulatorControllerMap::instance()->removeController(mId);
        mWaveDataList->deleteLater();
        delete mSimulationInput;
      //  delete mTempDir;
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

    std::string NetlistSimulatorController::get_working_directory() const
    {
        return mTempDir->path().toStdString();
    }

    std::filesystem::path NetlistSimulatorController::get_saleae_directory_filename() const
    {
        return std::filesystem::path(mWaveDataList->saleaeDirectory().get_filename());
    }

    SimulationEngine* NetlistSimulatorController::create_simulation_engine(const std::string& name)
    {
        SimulationEngineFactory* fac = SimulationEngineFactories::instance()->factoryByName(name);
        if (!fac) return nullptr;
        if (mSimulationEngine) delete mSimulationEngine;
        mSimulationEngine = fac->createEngine();
        mSimulationEngine->set_working_directory(get_working_directory());
        log_info(get_name(), "Engine '{}' created. Work directory set to '{}'.", mSimulationEngine->name(), get_working_directory());
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
        QVector<WaveData*> waveVector;
        waveVector.reserve(nets.size());
        for (Net* n : nets)
        {
            WaveData* wd = get_waveform_by_net(n);
            if (!wd)
            {
                log_warning(get_name(), "Cannot add unkown waveform for net '{}(id={})' to group '{}'.", n->get_name(), n->get_id(), name);
                continue;
            }
            waveVector.append(wd);
        }
        u32 grpId = mWaveDataList->createGroup(QString::fromStdString(name));
        mWaveDataList->addWavesToGroup(grpId, waveVector);
        return grpId;
    }

    void NetlistSimulatorController::remove_waveform_group(u32 group_id)
    {
        mWaveDataList->removeGroup(group_id);
    }

    void NetlistSimulatorController::handleOpenInputFile(const QString &filename)
    {
        if (filename.isEmpty()) return;
        VcdSerializer reader(this);
        QList<const Net*> onlyNets;
        for (const Net* n : mSimulationInput->get_input_nets()) onlyNets.append(n);
        if (reader.importVcd(filename,mTempDir->path(),onlyNets))
        {
            mWaveDataList->updateFromSaleae();
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
                WaveDataClock* wdc = new WaveDataClock(n, clk, mWaveDataList->timeFrame().sceneMaxTime());
                mWaveDataList->addOrReplace(wdc);
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
        mWaveDataList->triggerAddToView(n->get_id());
        return mWaveDataList->waveDataByNet(n);
    }

    WaveDataGroup* NetlistSimulatorController::get_waveform_group_by_id(u32 id) const
    {
        return mWaveDataList->mDataGroups.value(id);
    }

    void NetlistSimulatorController::rename_waveform(WaveData* wd, std::string name)
    {
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(wd);
        if (grp)
        {
            grp->rename(QString::fromStdString(name));
            mWaveDataList->emitGroupUpdated(grp->id());
            return;
        }
        int iwave = mWaveDataList->waveIndexByNetId(wd->id());
        if (iwave >= 0)
            mWaveDataList->updateWaveName(iwave, QString::fromStdString(name));
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

    void NetlistSimulatorController::emitLoadProgress(int percent)
    {
        Q_EMIT loadProgress(percent);
    }

    bool NetlistSimulatorController::import_vcd(const std::string& filename, FilterInputFlag filter)
    {
        VcdSerializer reader(this);

        QList<const Net*> inputNets;
        if (filter != NoFilter)
            for (const Net* n: getFilterNets(filter)) inputNets.append(n);

        if (reader.importVcd(QString::fromStdString(filename),mTempDir->path(),inputNets))
        {
            mWaveDataList->updateFromSaleae();
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
        VcdSerializer reader(this);

        QList<const Net*> inputNets;
        if (filter != NoFilter)
            for (const Net* n: getFilterNets(filter)) inputNets.append(n);

        if (reader.importCsv(QString::fromStdString(filename),mTempDir->path(),inputNets,timescale))
        {
            mWaveDataList->updateFromSaleae();
        }
        checkReadyState();
        Q_EMIT parseComplete();
    }

    void NetlistSimulatorController::import_saleae(const std::string& dirname, std::unordered_map<Net*,int> lookupTable, u64 timescale)
    {
        VcdSerializer reader(this);
        if (reader.importSaleae(QString::fromStdString(dirname),lookupTable,mTempDir->path(),timescale))
        {
            mWaveDataList->updateFromSaleae();
        }
        checkReadyState();
        Q_EMIT parseComplete();
    }

    void NetlistSimulatorController::import_simulation(const std::string& dirname, FilterInputFlag filter, u64 timescale)
    {
        QDir sourceDir(QString::fromStdString(dirname));
        QString sourceSaleaeLookup = sourceDir.absoluteFilePath("saleae.json");
        SaleaeDirectory sd(sourceSaleaeLookup.toStdString());
        if (!QFileInfo(sourceSaleaeLookup).isReadable())
        {
            log_warning(get_name(), "cannot import SALEAE data from '{}', cannot read lookup table.", dirname);
            return;
        }
        if (filter == NoFilter)
        {
            QDir targetDir(QString::fromStdString(mWaveDataList->saleaeDirectory().get_directory()));
            QString targetSaleaeLookup = targetDir.absoluteFilePath("saleae.json");
            QFile::remove(targetSaleaeLookup);
            QFile::copy(sourceSaleaeLookup,targetSaleaeLookup);
            QStringList nameFilters;
            nameFilters << "digital_*.bin";
            for (QFileInfo sourceFileInfo : sourceDir.entryInfoList(nameFilters))
            {
                QString targetFile = targetDir.absoluteFilePath(sourceFileInfo.fileName());
                QFile::remove(targetFile);
                QFile::copy(sourceFileInfo.absoluteFilePath(),targetFile);
            }
            mWaveDataList->updateFromSaleae();
        }
        else
        {
            std::unordered_map<Net*,int> lookupTable;
            for (const Net* n: getFilterNets(filter))
            {
                int inx = sd.get_datafile_index(n->get_name(),n->get_id());
                if (inx < 0) continue;
                lookupTable.insert(std::make_pair((Net*)n,inx));
            }
            VcdSerializer reader(this);
            if (reader.importSaleae(QString::fromStdString(dirname),lookupTable,mTempDir->path(),timescale))
            {
                mWaveDataList->updateFromSaleae();
            }
        }
        checkReadyState();
        for (const SaleaeDirectoryGroupEntry& sdge : sd.get_groups())
        {
            QVector<WaveData*> wds;
            for (const SaleaeDirectoryNetEntry& sdne : sdge.get_nets())
            {
                int iwave = mWaveDataList->waveIndexByNetId(sdne.id());
                if (iwave >= 0) wds.append(mWaveDataList->at(iwave));
            }
            if (!wds.isEmpty() && wds.size() == (int) sdge.get_nets().size())
            {
                u32 grpId = mWaveDataList->createGroup(QString::fromStdString(sdge.name()));
                mWaveDataList->addWavesToGroup(grpId, wds);
            }
        }
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
                    wd->insertBooleanValueWithoutSync(evt.time,evt.new_value);
                }
                mWaveDataList->addOrReplace(wd);
            }
        }
        else
        {
            std::filesystem::path resultFile = mSimulationEngine->get_result_filename();
            if (resultFile.is_relative())
                resultFile = get_working_directory() / resultFile;
            VcdSerializer reader(this);
            QFileInfo info(QString::fromStdString(resultFile.string()));
            if (!info.exists() || !info.isReadable()) return false;

            QList<const Net*> partialNets;
            for (const Net* n : get_partial_netlist_nets())
                partialNets.append(n);

            if (reader.importVcd(QString::fromStdString(resultFile),mTempDir->path(),partialNets))
            {
                mWaveDataList->updateFromSaleae();
            }
            else
                return false;
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

        if (mSimulationInput->is_ready() && mSimulationEngine && mWaveDataList->timeFrame().simulateMaxTime() > 0)
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
        if (!mSimulationInput->is_input_net(net))
        {
            if (mBadAssignInputWarnings[net->get_id()]++ < 3)
                log_warning(get_name(), "net[{}] '{}' is not an input net, value not assigned.", net->get_id(), net->get_name());
            return;
        }
        u64 t = mWaveDataList->timeFrame().simulateMaxTime();
        WaveData* wd = mWaveDataList->waveDataByNet(net);
        if (!wd)
        {
            wd = new WaveData(net);
            wd->insertBooleanValueWithoutSync(t,value);
            mWaveDataList->addOrReplace(wd);
        }
        else
            mWaveDataList->insertBooleanValue(wd,t,value);
    }

    void NetlistSimulatorController::set_timeframe(u64 tmin, u64 tmax)
    {
        mWaveDataList->setUserTimeframe(tmin, tmax);
    }

    Result<std::vector<u32>> NetlistSimulatorController::trace_value_if(const u32 group_id, const Net* trigger_net, const std::vector<Net*>& enable_nets, BooleanFunction enable_condition, u32 start_time, u32 end_time) const
    {
        const WaveDataGroup* const group = mWaveDataList->mDataGroups.value(group_id);
        QList<const WaveData*> partialList = mWaveDataList->partialList(start_time, end_time, {trigger_net});
        const auto enable_vars             = enable_condition.get_variable_names();
        if (std::any_of(enable_nets.begin(), enable_nets.end(), [enable_vars](const Net* n) { return enable_vars.find(n->get_name()) == enable_vars.end(); }))
        {
            return ERR("could not record values of waveform group with ID " + std::to_string(group->id()) + ": not all variables of Boolean functions contained within provided nets");
        }

        std::map<std::string, WaveData*> enable_waves;
        for (const auto* n : enable_nets) 
        {
            enable_waves[n->get_name()] = mWaveDataList->waveDataByNet(n);
        }

        std::vector<u32> trace;
        for (auto [time, value] : partialList.at(0)->get_events())
        {
            UNUSED(value);
            std::unordered_map<std::string, BooleanFunction::Value> values;
            for (const auto& [var, wave] : enable_waves) 
            {
                values[var] = (BooleanFunction::Value) wave->get_value_at(time);
            }
            
            if (auto res = enable_condition.evaluate(values); res.is_error()) 
            {
                return ERR("could not record values of waveform group with ID " + std::to_string(group->id()) + ": failed to evaluate 'enable' function");
            } 
            else 
            {
                trace.push_back((u32)group->get_value_at(time));
            }
        }
        return OK(trace);
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

}    // namespace hal
