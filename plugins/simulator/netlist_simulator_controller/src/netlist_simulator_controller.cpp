#include "netlist_simulator_controller/netlist_simulator_controller.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/json_write_document.h"
#include "hal_core/utilities/log.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/saleae_parser.h"
#include "netlist_simulator_controller/simulation_engine.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "netlist_simulator_controller/vcd_serializer.h"
#include "netlist_simulator_controller/wave_data.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
#include <QVector>

namespace hal
{
    const char* NetlistSimulatorController::sPersistFile = "netlist_simulator_controller.json";

    NetlistSimulatorController::NetlistSimulatorController(u32 id, const std::string nam, const std::string& workdir, QObject* parent)
        : QObject(parent), mId(id), mName(QString::fromStdString(nam)), mState(NoGatesSelected), mSimulationEngine(nullptr), mTempDir(nullptr), mWaveDataList(nullptr),
          mSimulationInput(new SimulationInput)
    {
        if (mName.isEmpty())
            mName = QString("sim_controller%1").arg(mId);
        LogManager::get_instance()->add_channel(mName.toStdString(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");

        if (workdir.empty())
        {
            QString templatePath = NetlistSimulatorControllerPlugin::sSimulationSettings->baseDirectory().isEmpty() ? QString::fromStdString(ProjectManager::instance()->get_project_directory())
                                                                                                                    : NetlistSimulatorControllerPlugin::sSimulationSettings->baseDirectory();
            if (!templatePath.isEmpty())
                templatePath += '/';
            templatePath += "hal_simulation_" + mName + "_XXXXXX";
            mTempDir = new QTemporaryDir(templatePath);
            mWorkDir = mTempDir->path();
        }
        else
            mWorkDir = QString::fromStdString(workdir);
        QDir saleaeDir(QDir(mWorkDir).absoluteFilePath("saleae"));
        saleaeDir.mkpath(saleaeDir.absolutePath());
        QString saleaeDirectoryFilename = saleaeDir.absoluteFilePath("saleae.json");
        if (!QFileInfo(saleaeDirectoryFilename).exists())
        {
            QFile of(saleaeDirectoryFilename);
            if (of.open(QIODevice::WriteOnly))
                of.write(QByteArray("{\"saleae\":{}}"));
        }
        mWaveDataList = new WaveDataList(saleaeDirectoryFilename);

        NetlistSimulatorControllerMap::instance()->addController(this);
    }

    NetlistSimulatorController::NetlistSimulatorController(u32 id, Netlist* nl, const std::string& filename, QObject* parent)
        : QObject(parent), mId(id), mState(NoGatesSelected), mSimulationEngine(nullptr), mTempDir(nullptr), mWaveDataList(nullptr), mSimulationInput(new SimulationInput)
    {
        FILE* ff = fopen(filename.c_str(), "rb");
        if (!ff)
        {
            log_warning("simulation_plugin", "Error opening file '{}'.", filename);
            return;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(ff, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(ff);

        if (document.HasParseError() || !document.HasMember("netlist_simulator_controller"))
        {
            log_warning("simulation_plugin", "Cannot restore simulation controller from file '{}'.", filename);
            return;
        }
        auto jnsc = document["netlist_simulator_controller"].GetObject();
        if (jnsc.HasMember("name"))
            mName = QString::fromStdString(jnsc["name"].GetString());

        QDir workDir(QFileInfo(QString::fromStdString(filename)).path());
        std::vector<Gate*> simulatedGates;
        if (jnsc.HasMember("gates"))
        {
            for (auto& jgate : jnsc["gates"].GetArray())
            {
                u32 gateId = jgate.HasMember("id") ? jgate["id"].GetUint() : 0;
                Gate* g    = nl->get_gate_by_id(gateId);
                if (!g)
                {
                    log_warning("simulation_plugin", "Simulated gate ID={} not found in netlist.", gateId);
                    return;
                }
                if (jgate.HasMember("name") && jgate["name"].GetString() != g->get_name())
                {
                    log_warning("simulation_plugin", "Gate name for ID={} differs in simulation '{}' and netlist '{}'", gateId, jgate["name"].GetString(), g->get_name());
                    return;
                }
                simulatedGates.push_back(g);
            }
        }
        LogManager::get_instance()->add_channel(mName.toStdString(), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        QDir saleaeDir(workDir.absoluteFilePath("saleae"));
        saleaeDir.mkpath(saleaeDir.absolutePath());
        mWaveDataList = new WaveDataList(saleaeDir.absoluteFilePath("saleae.json"));

        SaleaeDirectoryStoreRequest::sWriteDisabled = true;
        mWaveDataList->updateFromSaleae();
        mSimulationInput->add_gates(simulatedGates);
        mWorkDir = workDir.absolutePath();
        restoreComposed(mWaveDataList->saleaeDirectory());
        SaleaeDirectoryStoreRequest::sWriteDisabled = false;

        if (jnsc.HasMember("clocks"))
        {
            for (auto& jclock : jnsc["clocks"].GetArray())
            {
                u32 clkId   = jclock.HasMember("id") ? jclock["id"].GetUint() : 0;
                Net* clkNet = nl->get_net_by_id(clkId);
                if (!clkNet)
                {
                    log_warning(mName.toStdString(), "Clock net ID={} not found in netlist.", clkId);
                    continue;
                }
                bool startAtZero = jclock.HasMember("start_value") ? (jclock["start_value"].GetInt() == 0) : true;
                int period       = jclock.HasMember("switch_time") ? jclock["switch_time"].GetInt() * 2 : 1000;
                add_clock_period(clkNet, period, startAtZero, mWaveDataList->timeFrame().simulateMaxTime());
            }
        }

        if (jnsc.HasMember("engine"))
        {
            auto jengine = jnsc["engine"].GetObject();
            if (jengine.HasMember("name"))
            {
                create_simulation_engine(jengine["name"].GetString());
                if (mSimulationEngine && jengine.HasMember("properties"))
                {
                    auto jprop = jengine["properties"].GetObject();

                    for (auto it = jprop.MemberBegin(); it != jprop.MemberEnd(); ++it)
                    {
                        mSimulationEngine->set_engine_property(it->name.GetString(), it->value.GetString());
                    }
                }
            }
        }
        NetlistSimulatorControllerMap::instance()->addController(this);
    }

    NetlistSimulatorController::~NetlistSimulatorController()
    {
        NetlistSimulatorControllerMap::instance()->removeController(mId);
        if (mWaveDataList)
            mWaveDataList->deleteLater();
        delete mSimulationInput;
        //  delete mTempDir;
    }

    void NetlistSimulatorController::setState(SimulationState stat)
    {
        if (stat == mState)
            return;
        mState = stat;
        switch (mState)
        {
            case NoGatesSelected:
                log_info(get_name(), "Select gates for simulation");
                break;
            case ParameterSetup:
                log_info(get_name(), "Expecting parameter and input");
                break;
            case ParameterReady:
                log_info(get_name(), "Preconditions to start simulation met");
                break;
            case SimulationRun:
                log_info(get_name(), "Running simulation, please wait...");
                break;
            case ShowResults:
                log_info(get_name(), "Simulation engine completed successfully");
                break;
            case EngineFailed:
                log_info(get_name(), "Simulation engine process error");
                if (mSimulationEngine)
                    mSimulationEngine->failed();
                break;
        }
        Q_EMIT stateChanged(mState);
    }

    std::string NetlistSimulatorController::get_working_directory() const
    {
        return mWorkDir.toStdString();
    }

    bool NetlistSimulatorController::is_legal_directory_name() const
    {
        if (mWorkDir.contains(' '))
            return false;
        return true;
    }

    u64 NetlistSimulatorController::get_max_simulated_time() const
    {
        if (!mWaveDataList) return 0;
        return mWaveDataList->timeFrame().simulateMaxTime();
    }

    std::filesystem::path NetlistSimulatorController::get_saleae_directory_filename() const
    {
        return std::filesystem::path(mWaveDataList->saleaeDirectory().get_filename());
    }

    SimulationEngine* NetlistSimulatorController::create_simulation_engine(const std::string& name)
    {
        SimulationEngineFactory* fac = SimulationEngineFactories::instance()->factoryByName(name);
        if (!fac)
            return nullptr;
        if (mSimulationEngine)
            delete mSimulationEngine;
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

    u32 NetlistSimulatorController::add_trigger_time(const std::vector<WaveData*>& trigger_waves, const std::vector<int>& trigger_on_values)
    {
        if (trigger_waves.empty())
            return 0;
        QList<WaveData*> triglist;
        QList<int> trigOnVal;
        for (WaveData* wd : trigger_waves)
            triglist.append(wd);
        for (int tov : trigger_on_values)
            trigOnVal.append(tov);
        WaveDataTrigger* wdTrig = new WaveDataTrigger(mWaveDataList, triglist, trigOnVal);
        if (!wdTrig)
            return 0;
        return wdTrig->id();
    }

    u32 NetlistSimulatorController::add_boolean_expression_waveform(const std::string& expression)
    {
        if (expression.empty())
            return 0;
        WaveDataBoolean* wdBool = new WaveDataBoolean(mWaveDataList, QString::fromStdString(expression));
        if (!wdBool)
            return 0;
        return wdBool->id();
    }

    u32 NetlistSimulatorController::add_boolean_accept_list_waveform(const std::vector<WaveData*>& input_waves, const std::vector<int>& accepted_combination)
    {
        if (input_waves.empty() || accepted_combination.empty())
            return 0;
        QList<WaveData*> inpWaves;
        QList<int> acceptVal;
        for (WaveData* wd : input_waves)
            inpWaves.append(wd);
        for (int acc : accepted_combination)
            acceptVal.append(acc);
        WaveDataBoolean* wdBool = new WaveDataBoolean(mWaveDataList, inpWaves, acceptVal);
        if (!wdBool)
            return 0;
        return wdBool->id();
    }

    u32 NetlistSimulatorController::add_waveform_group(const std::string& name, const std::vector<Net*>& nets)
    {
        if (name.empty())
            return 0;
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

        WaveDataGroup* wdGrp = new WaveDataGroup(mWaveDataList, QString::fromStdString(name));
        mWaveDataList->addWavesToGroup(wdGrp->id(), waveVector);
        return wdGrp->id();
    }

    u32 NetlistSimulatorController::add_waveform_group(const PinGroup<ModulePin>* pin_group)
    {
        std::vector<Net*> nets;
        for (const auto pin : pin_group->get_pins())
        {
            nets.push_back(pin->get_net());
        }

        return add_waveform_group(pin_group->get_name(), nets);
    }

    u32 NetlistSimulatorController::add_waveform_group(const std::string& name, const PinGroup<ModulePin>* pin_group)
    {
        std::vector<Net*> nets;
        for (const auto pin : pin_group->get_pins())
        {
            nets.push_back(pin->get_net());
        }

        return add_waveform_group(name, nets);
    }

    void NetlistSimulatorController::remove_waveform_group(u32 group_id)
    {
        mWaveDataList->removeGroup(group_id);
    }

    void NetlistSimulatorController::handleOpenInputFile(const QString& filename)
    {
        if (filename.isEmpty())
            return;
        VcdSerializer reader(mWorkDir, this);
        QList<const Net*> onlyNets;
        for (const Net* n : mSimulationInput->get_input_nets())
            onlyNets.append(n);
        if (reader.importVcd(filename, mWorkDir, onlyNets))
        {
            mWaveDataList->updateFromSaleae();
        }
        checkReadyState();
    }

    bool NetlistSimulatorController::persist() const
    {
        JsonWriteDocument jwd;
        JsonWriteObject& jnsc = jwd.add_object("netlist_simulator_controller");
        jnsc["id"]            = (int)get_id();
        jnsc["name"]          = get_name();
        /*       jnsc["workdir"] = get_working_directory();*/

        JsonWriteArray& jgates = jnsc.add_array("gates");
        for (const Gate* g : mSimulationInput->get_gates())
        {
            JsonWriteObject& jgate = jgates.add_object();
            jgate["id"]            = (int)g->get_id();
            jgate["name"]          = g->get_name();
            jgate.close();
        }
        jgates.close();

        JsonWriteArray& jclocks = jnsc.add_array("clocks");
        for (const SimulationInput::Clock& clk : mSimulationInput->get_clocks())
        {
            JsonWriteObject& jclock = jclocks.add_object();
            jclock["id"]            = (int)clk.clock_net->get_id();
            jclock["name"]          = clk.clock_net->get_name();
            jclock["switch_time"]   = (int)clk.switch_time;
            jclock["start_value"]   = clk.start_at_zero ? 0 : 1;
            jclock.close();
        }
        jclocks.close();

        if (mSimulationEngine)
        {
            JsonWriteObject& jengine = jnsc.add_object("engine");
            jengine["name"]          = mSimulationEngine->name();
            JsonWriteObject& jprops  = jengine.add_object("properties");
            for (auto it = mSimulationEngine->get_engine_properties().begin(); it != mSimulationEngine->get_engine_properties().end(); ++it)
            {
                jprops[it->first] = it->second;
            }
            jprops.close();
            jengine.close();
        }

        jnsc.close();
        return jwd.serialize(QDir(mWorkDir).absoluteFilePath(sPersistFile).toStdString());
    }

    void NetlistSimulatorController::emit_run_simulation()
    {
        Q_EMIT triggerRunSimulation();
    }

    bool NetlistSimulatorController::run_simulation()
    {
        if (!mSimulationEngine)
        {
            log_warning(get_name(), "no simulation engine selected");
            return false;
        }

        QMap<QString, QString> engPropMap = NetlistSimulatorControllerPlugin::sSimulationSettings->engineProperties();

        if (!engPropMap.isEmpty())
        {
            bool engPropMapModified = false;
            for (auto it = engPropMap.begin(); it != engPropMap.end(); ++it)
            {
                std::string prop = it.key().toStdString();
                std::string valu = it.value().toStdString();

                std::string userAssignedValue = mSimulationEngine->get_engine_property(prop);
                if (userAssignedValue.empty())
                {
                    log_info(get_name(), "Engine property '{}' set to '{}'.", prop, valu);
                    mSimulationEngine->set_engine_property(prop, valu);
                }
                else
                {
                    if (userAssignedValue != valu)
                    {
                        log_info(get_name(), "Default value for engine property '{}' changed from '{}' to '{}'.", prop, valu, userAssignedValue);
                        it.value() = QString::fromStdString(userAssignedValue);
                        engPropMapModified = true;
                    }
                }
            }
            if (engPropMapModified)
            {
                NetlistSimulatorControllerPlugin::sSimulationSettings->setEngineProperties(engPropMap);
                NetlistSimulatorControllerPlugin::sSimulationSettings->sync();
            }
        }

        if (mState != ParameterReady)
        {
            log_warning(get_name(), "wrong state {}.", (u32)mState);
            return false;
        }

        mWaveDataList->setValueForEmpty(0);
        mWaveDataList->emitTimeframeChanged();
        qApp->processEvents();

        for (auto it = mBadAssignInputWarnings.constBegin(); it != mBadAssignInputWarnings.constEnd(); ++it)
            if (it.value() > 3)
                log_warning(get_name(), "Totally {} attempts to set input values for net ID={}, but net is not an input.", it.value(), it.key());

        struct WaveIterator
        {
            const Net* n;
            const WaveData* wd;
            QMap<u64, int>::const_iterator it;
        };

        // generate clock events if required
        if (mSimulationEngine->clock_events_required())
        {
            for (const Net* n : mSimulationInput->get_input_nets())
            {
                if (!mSimulationInput->is_clock(n))
                    continue;
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

        qApp->processEvents();
        persist();

        if (!mSimulationEngine->setSimulationInput(mSimulationInput))
        {
            log_warning(get_name(), "simulation engine error during setup.");
            setState(EngineFailed);
            return false;
        }

        // start simulation process (might be external process)
        if (!mSimulationEngine->run(this))
        {
            log_warning(get_name(), "simulation engine error during startup.");
            setState(EngineFailed);
            return false;
        }
        setState(SimulationRun);
        return true;
    }

    WaveData* NetlistSimulatorController::get_waveform_by_net(const Net* n) const
    {
        mWaveDataList->triggerAddToView(n->get_id());
        return mWaveDataList->waveDataByNet(n);
    }

    WaveDataGroup* NetlistSimulatorController::get_waveform_group_by_id(u32 id) const
    {
        return mWaveDataList->mDataGroups.value(id);
    }

    WaveDataBoolean* NetlistSimulatorController::get_waveform_boolean_by_id(u32 id) const
    {
        return mWaveDataList->mDataBooleans.value(id);
    }

    WaveDataTrigger* NetlistSimulatorController::get_trigger_time_by_id(u32 id) const
    {
        return mWaveDataList->mDataTrigger.value(id);
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

    std::vector<const Net*> NetlistSimulatorController::getFilterNets(FilterInputFlag filter) const
    {
        if (!mSimulationInput->has_gates())
            return std::vector<const Net*>();
        switch (filter)
        {
            case GlobalInputs:
                return std::vector<const Net*>(get_input_nets().begin(), get_input_nets().end());
            case PartialNetlist:
                return get_partial_netlist_nets();
            case CompleteNetlist: {
                std::vector<Net*> tmp = (*mSimulationInput->get_gates().begin())->get_netlist()->get_nets();
                return std::vector<const Net*>(tmp.begin(), tmp.end());
            }
            case NoFilter:
                break;
        }
        return std::vector<const Net*>();
    }

    bool NetlistSimulatorController::can_import_data() const
    {
        // TODO : check for ongoing import ?
        if (mState == ParameterReady || mState == ParameterSetup || mState == ShowResults)
            return true;
        return false;
    }

    void NetlistSimulatorController::emitLoadProgress(int percent)
    {
        Q_EMIT loadProgress(percent);
    }

    bool NetlistSimulatorController::import_vcd(const std::string& filename, FilterInputFlag filter)
    {
        VcdSerializer reader(mWorkDir, this);

        QList<const Net*> inputNets;
        if (filter != NoFilter)
            for (const Net* n : getFilterNets(filter))
                inputNets.append(n);

        if (reader.importVcd(QString::fromStdString(filename), mWorkDir, inputNets))
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
        VcdSerializer reader(mWorkDir, this);

        QList<const Net*> inputNets;
        if (filter != NoFilter)
            for (const Net* n : getFilterNets(filter))
                inputNets.append(n);

        if (reader.importCsv(QString::fromStdString(filename), mWorkDir, inputNets, timescale))
        {
            mWaveDataList->updateFromSaleae();
        }
        checkReadyState();
        Q_EMIT parseComplete();
    }

    void NetlistSimulatorController::import_saleae(const std::string& dirname, std::unordered_map<Net*, int> lookupTable, u64 timescale)
    {
        VcdSerializer reader(mWorkDir, this);
        if (reader.importSaleae(QString::fromStdString(dirname), lookupTable, mWorkDir, timescale))
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
            QFile::copy(sourceSaleaeLookup, targetSaleaeLookup);
            QStringList nameFilters;
            nameFilters << "digital_*.bin";
            for (QFileInfo sourceFileInfo : sourceDir.entryInfoList(nameFilters))
            {
                QString targetFile = targetDir.absoluteFilePath(sourceFileInfo.fileName());
                QFile::remove(targetFile);
                QFile::copy(sourceFileInfo.absoluteFilePath(), targetFile);
            }
            mWaveDataList->updateFromSaleae();
        }
        else
        {
            std::unordered_map<Net*, int> lookupTable;
            for (const Net* n : getFilterNets(filter))
            {
                int inx = sd.get_datafile_index(n->get_name(), n->get_id());
                if (inx < 0)
                    continue;
                lookupTable.insert(std::make_pair((Net*)n, inx));
            }
            VcdSerializer reader(mWorkDir, this);
            if (reader.importSaleae(QString::fromStdString(dirname), lookupTable, mWorkDir, timescale))
            {
                mWaveDataList->updateFromSaleae();
            }
        }
        checkReadyState();
        restoreComposed(sd);
        Q_EMIT parseComplete();
    }

    void NetlistSimulatorController::restoreComposed(const SaleaeDirectory& sd)
    {
        for (const SaleaeDirectoryComposedEntry& sdce : sd.get_composed_list())
        {
            sdce.dump();
            QVector<WaveData*> wds;
            for (int childKey : sdce.get_children())
            {
                WaveData* wd = nullptr;
                int mType    = childKey / SaleaeDirectoryNetEntry::sComposedBaseKey;
                int index    = childKey % SaleaeDirectoryNetEntry::sComposedBaseKey;
                switch (mType)
                {
                    case SaleaeDirectoryNetEntry::Group:
                        wd = mWaveDataList->mDataGroups.value(index);
                        break;
                    case SaleaeDirectoryNetEntry::Boolean:
                        wd = mWaveDataList->mDataBooleans.value(index);
                        break;
                    case SaleaeDirectoryNetEntry::Trigger:
                        wd = mWaveDataList->mDataTrigger.value(index);
                        break;
                    default:
                        int iwave = mWaveDataList->waveIndexByNetId(childKey);
                        if (iwave >= 0)
                            wd = mWaveDataList->at(iwave);
                }
                if (wd)
                    wds.append(wd);
            }
            QList<int> data;
            for (int dat : sdce.get_data())
            {
                data.append(dat);
            }
            if (!wds.isEmpty() && wds.size() == (int)sdce.get_children().size())
            {
                switch (sdce.type())
                {
                    case SaleaeDirectoryNetEntry::Group: {
                        WaveDataGroup* wdGrp = new WaveDataGroup(mWaveDataList, QString::fromStdString(sdce.name()));
                        mWaveDataList->addWavesToGroup(wdGrp->id(), wds);
                        break;
                    }
                    case SaleaeDirectoryNetEntry::Boolean: {
                        new WaveDataBoolean(mWaveDataList, wds.toList(), data);
                        break;
                    }
                    case SaleaeDirectoryNetEntry::Trigger: {
                        WaveDataTrigger* wdTrig = new WaveDataTrigger(mWaveDataList, wds.toList(), data);
                        if (sdce.get_filter_entry())
                        {
                            int filterKey = sdce.get_filter_entry();
                            if (filterKey > 0)
                            {
                                WaveData* wd = nullptr;
                                int mType    = filterKey / SaleaeDirectoryNetEntry::sComposedBaseKey;
                                int index    = filterKey % SaleaeDirectoryNetEntry::sComposedBaseKey;
                                switch (mType)
                                {
                                    case SaleaeDirectoryNetEntry::Group:
                                        wd = mWaveDataList->mDataGroups.value(index);
                                        break;
                                    case SaleaeDirectoryNetEntry::Boolean:
                                        wd = mWaveDataList->mDataBooleans.value(index);
                                        break;
                                    case SaleaeDirectoryNetEntry::Trigger:
                                        wd = mWaveDataList->mDataTrigger.value(index);
                                        break;
                                    default:
                                        int iwave = mWaveDataList->waveIndexByNetId(filterKey);
                                        if (iwave >= 0)
                                            wd = mWaveDataList->at(iwave);
                                }
                                if (wd)
                                    wdTrig->set_filter_wave(wd);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
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
                    wd->insertBooleanValueWithoutSync(evt.time, evt.new_value);
                }
                mWaveDataList->addOrReplace(wd);
            }
        }
        else
        {
            std::filesystem::path resultFile = mSimulationEngine->get_result_filename();
            if (resultFile.is_relative())
                resultFile = get_working_directory() / resultFile;
            VcdSerializer reader(mWorkDir, this);
            QFileInfo info(QString::fromStdString(resultFile.string()));
            if (!info.exists() || !info.isReadable())
                return false;

            QList<const Net*> partialNets;
            for (const Net* n : get_partial_netlist_nets())
                partialNets.append(n);

            if (reader.importVcd(QString::fromStdString(resultFile), mWorkDir, partialNets))
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
        if (mState >= ParameterReady)
            return;    // nothing to do

        if (mSimulationInput->is_ready() && mSimulationEngine && mWaveDataList->timeFrame().simulateMaxTime() > 0)
            setState(ParameterReady);
        persist();
    }

    void NetlistSimulatorController::add_clock_period(const Net* clock_net, u64 period, bool start_at_zero, u64 duration)
    {
        if (!period)
        {
            log_warning(get_name(), "Generating clock failed, period must not be zero!");
            return;
        }
        SimulationInput::Clock clk;
        clk.clock_net     = clock_net;
        clk.switch_time   = period / 2;
        clk.start_at_zero = start_at_zero;
        mSimulationInput->add_clock(clk);
        WaveData* wd = new WaveDataClock(clock_net, clk, duration ? duration : 2000);
        mWaveDataList->addOrReplace(wd);
        checkReadyState();
    }

    void NetlistSimulatorController::add_gates(const std::vector<Gate*>& gates)
    {
        if (mState != NoGatesSelected)
        {
            log_warning(get_name(), "Command failed, gates for simulation already selected in this controller.");
            return;
        }
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
        if (mState == NoGatesSelected && mSimulationInput->has_gates())
            setState(ParameterSetup);
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
        u64 t        = mWaveDataList->timeFrame().simulateMaxTime();
        WaveData* wd = mWaveDataList->waveDataByNet(net);
        if (!wd)
        {
            wd = new WaveData(net);
            wd->insertBooleanValueWithoutSync(t, value);
            mWaveDataList->addOrReplace(wd);
        }
        else
            mWaveDataList->insertBooleanValue(wd, t, value);
    }

    void NetlistSimulatorController::set_input(WaveData* wd, BooleanFunction::Value value)
    {
        u64 t = mWaveDataList->timeFrame().simulateMaxTime();
        mWaveDataList->insertBooleanValue(wd, t, value);
    }

    void NetlistSimulatorController::set_input(const std::vector<Net*>& nets, const std::vector<BooleanFunction::Value>& values)
    {
        if (nets.size() != values.size())
        {
            log_error(get_name(), "Cannot set vector of nets to vector of values, because vectors are not of equal size! {} vs. {}", nets.size(), values.size());
            return;
        }

        for (u32 idx = 0; idx < nets.size(); idx++)
        {
            set_input(nets.at(idx), values.at(idx));
        }
    }

    void NetlistSimulatorController::set_input(const WaveDataGroup* wdg, const std::vector<BooleanFunction::Value>& values)
    {
        const auto wave_forms = wdg->get_waveforms();
        if (wave_forms.size() != values.size())
        {
            log_error(
                get_name(), "Cannot set WaveDataGroup to vector of values, because the amount of grouped wave forms is not equal to the vector size! {} vs. {}", wave_forms.size(), values.size());
            return;
        }

        for (u32 idx = 0; idx < wave_forms.size(); idx++)
        {
            set_input(wave_forms.at(idx), values.at(idx));
        }
    }

    void NetlistSimulatorController::set_input(const u32 id, const std::vector<BooleanFunction::Value>& values)
    {
        const auto wave_data_group = get_waveform_group_by_id(id);
        set_input(wave_data_group, values);
    }

    void NetlistSimulatorController::set_input(const PinGroup<ModulePin>* pin_group, const std::vector<BooleanFunction::Value>& values)
    {
        std::vector<Net*> nets;
        for (const auto pin : pin_group->get_pins())
        {
            nets.push_back(pin->get_net());
        }

        set_input(nets, values);
    }

    void NetlistSimulatorController::set_timeframe(u64 tmin, u64 tmax)
    {
        mWaveDataList->setUserTimeframe(tmin, tmax);
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
        VcdSerializer writer(mWorkDir);
        QList<const WaveData*> partialList;
        if (nets.empty())
        {
            for (const WaveData* wd : *mWaveDataList)
                partialList.append(wd);
        }
        else
        {
            for (const Net* n : nets)
            {
                const WaveData* wd = mWaveDataList->waveDataByNet(n);
                if (wd)
                    partialList.append(wd);
            }
        }
        if (!start_time && !end_time)
        {
            start_time = mWaveDataList->timeFrame().sceneMinTime();
            end_time   = mWaveDataList->timeFrame().sceneMaxTime();
        }
        bool success = writer.exportVcd(QString::fromStdString(path.string()), partialList, start_time, end_time);
        return success;
    }

    NetlistSimulatorControllerMap* NetlistSimulatorControllerMap::sInst = nullptr;

    NetlistSimulatorControllerMap* NetlistSimulatorControllerMap::instance()
    {
        if (!sInst)
            sInst = new NetlistSimulatorControllerMap;
        return sInst;
    }

    void NetlistSimulatorControllerMap::addController(NetlistSimulatorController* ctrl)
    {
        u32 id = ctrl->get_id();
        mMap.insert(id, ctrl);
        Q_EMIT controllerAdded(id);
    }

    void NetlistSimulatorControllerMap::removeController(u32 id)
    {
        auto it = mMap.find(id);
        if (it == mMap.end())
            return;
        mMap.erase(it);
        Q_EMIT controllerRemoved(id);
    }

}    // namespace hal
