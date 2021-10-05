#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/simulation_engine.h"

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
#include <QDebug>
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"

namespace hal
{


    NetlistSimulatorController::NetlistSimulatorController(QObject *parent)
        : QObject(parent), mState(NoGatesSelected), mSimulationEngine(nullptr),
          mSimulationInput(new SimulationInput)
    {;}

    void NetlistSimulatorController::setState(SimulationState stat)
    {
        if (stat==mState) return;
        mState = stat;
        switch (mState)
        {
        case NoGatesSelected:  log_info("simulator_contr", "Select gates for simulation");            break;
        case ParameterSetup:   log_info("simulator_contr", "Expecting parameter and input");          break;
        case SimulationRun:    log_info("simulator_contr", "Running simulation, please wait...");     break;
        case ShowResults:      log_info("simulator_contr", "Select wires in graph to show results");  break;
        }
    }

    bool NetlistSimulatorController::setSimulationEngine(const QString& name)
    {
        mSimulationEngine = SimulationEngines::instance()->engineByName(name.toStdString());
        return (mSimulationEngine != nullptr);
    }

    SimulationEngine* NetlistSimulatorController::simulationEngine() const
    {
        return mSimulationEngine;
    }

    void NetlistSimulatorController::initSimulator()
    {
        /*
        NetlistSimulatorPlugin* simPlug = static_cast<NetlistSimulatorPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator"));
        if (!simPlug)
        {
            qDebug() << "Plugin 'netlist_simulator' not found";
            return;
        }
        qDebug() << "access to plugin" << simPlug->get_name().c_str() << simPlug->get_version().c_str();
        mSimulator = simPlug->get_shared_simulator("vcd_viewer");
        if (!mSimulator)
        {
            qDebug() << "Cannot create new simulator";
            return;
        }
        mSimulator->reset();
        qDebug() << "sim has gates " << mSimulator->get_gates().size();
        if (!gNetlist)
        {
            qDebug() << "No netlist loaded";
            return;
        }
        qDebug() << "net has gates " << gNetlist->get_gates().size();
        mSimulator->add_gates(mSimulateGates);


        mClkNet = nullptr;
        mInputNets = mSimulator->get_input_nets();
        for (const Net* n : mInputNets)
        {
            WaveData* wd = new WaveData(n);
            wd->insert(0,0);
        }
        mSimulator->set_iteration_timeout(1000);
        setState(ParameterSetup);
        */
    }


    void NetlistSimulatorController::handleSimulSettings()
    {
        /*
         act = new QAction("Select gates for simulation", settingMenu);
         connect(act, &QAction::triggered, this, &NetlistSimulatorController::handleSelectGates);
         act = new QAction("Select clock net", settingMenu);
         connect(act, &QAction::triggered, this, &NetlistSimulatorController::handleClockSet);
         act->setEnabled(mState==SimulationClockSet);
         act = new QAction("Visualize net state by color", settingMenu);
         act->setCheckable(true);
         act->setChecked(mWaveWidget->isVisulizeNetState());
         connect (act, &QAction::triggered, mWaveWidget, &WaveWidget::setVisualizeNetState);
*/
    }

    void NetlistSimulatorController::handleOpenInputFile(const QString &filename)
    {
        if (filename.isEmpty()) return;
        VcdSerializer reader;
        for (WaveData* wd : reader.deserialize(filename))
            mWaveDataList.addOrReplace(wd);
    }

    void NetlistSimulatorController::handleRunSimulation()
    {
        if (!mSimulationEngine)
        {
            log_warning("sim_controller", "no simulation engine selected");
            return;
        }

        mResultMap.clear();

        if (mState != ParameterSetup)
        {
            log_warning("sim_controller", "wrong state {}.", (u32) mState);
            return;
        }

        QMultiMap<int,QPair<const Net*, BooleanFunction::Value>> inputMap;
        for (const Net* n : mSimulationInput->get_input_nets())
        {
            if (mSimulationInput->is_clock(n)) continue;
            const WaveData* wd = mWaveDataList.waveDataByNetId(n->get_id());
            for (auto it=wd->constBegin(); it != wd->constEnd(); ++it)
            {
                BooleanFunction::Value sv;
                switch (it.value())
                {
                case -2: sv = BooleanFunction::Value::Z; break;
                case -1: sv = BooleanFunction::Value::X; break;
                case  0: sv = BooleanFunction::Value::ZERO; break;
                case  1: sv = BooleanFunction::Value::ONE; break;
                default: continue;
                }
                inputMap.insertMulti(it.key(),QPair<const Net*,BooleanFunction::Value>(n,sv));
            }
        }

        int t=0;
        SimulationInputNetEvent netEv;
        for (auto it = inputMap.begin(); it != inputMap.end(); ++it)
        {
            if (it.key() != t)
            {
                netEv.set_simulation_duration(it.key() - t);
                mSimulationInput->add_simulation_net_event(netEv);
                netEv.clear();
                t = it.key();
            }
            netEv[it.value().first] = it.value().second;
        }

        mSimulationEngine->setSimulationInput(mSimulationInput);

        mSimulationEngine->run();

        mSimulationEngine->done();
        /*
       for (Net* n : gNetlist->get_nets())
        {
            WaveData* wd = WaveData::simulationResultFactory(n, mSimulator.get());
            if (wd) mResultMap.insert(wd->id(),wd);
        }

        mSimulator->generate_vcd("result.vcd",0,t);

        VcdSerializer reader(this);
        for (WaveData* wd : reader.deserialize("result.vcd"))
            mResults.insert(wd->name(),wd);
            */
        qDebug() << "results" << mResultMap.size();
        setState(ShowResults);
    }

    void NetlistSimulatorController::add_clock_frequency(const Net* clock_net, u64 frequency, bool start_at_zero)
    {
        u64 period = 1'000'000'000'000ul / frequency;
        add_clock_period(clock_net, period, start_at_zero);
    }

    void NetlistSimulatorController::add_clock_period(const Net* clock_net, u64 period, bool start_at_zero)
    {
        SimulationInput::Clock clk;
        clk.clock_net     = clock_net;
        clk.switch_time   = period / 2;
        clk.start_at_zero = start_at_zero;
        mSimulationInput->add_clock(clk);
        WaveData* wd = WaveData::clockFactory(clock_net, start_at_zero ? 0 : 1, period, 2000);
        mWaveDataList.addOrReplace(wd);
    }

    void NetlistSimulatorController::add_gates(const std::vector<Gate *> &gates)
    {
        mSimulationInput->add_gates(gates);

        QSet<u32> previousInputSet = mWaveDataList.toSet();
        QSet<u32> currentInputSet;
        for (const Net* n : mSimulationInput->get_input_nets())
        {
            u32 nid = n->get_id();
            if (!previousInputSet.contains(nid))
            {
                WaveData* wd = new WaveData(nid, QString::fromStdString(n->get_name()));
                mWaveDataList.addOrReplace(wd);
            }
            currentInputSet.insert(nid);
        }
        previousInputSet -= currentInputSet;
        for (u32 id : previousInputSet)
        {
            mWaveDataList.remove(id);
        }
    }

    const std::unordered_set<const Gate*>& NetlistSimulatorController::get_gates() const
    {
        return mSimulationInput->get_gates();
    }

    const std::vector<const Net*>& NetlistSimulatorController::get_input_nets() const
    {
        return mSimulationInput->get_input_nets();
    }

    const std::vector<const Net*>& NetlistSimulatorController::get_output_nets() const
    {
        return mSimulationInput->get_output_nets();
    }

    void NetlistSimulatorController::set_input(const Net* net, BooleanFunction::Value value)
    {

    }

    void NetlistSimulatorController::initialize()
    {

    }

    void NetlistSimulatorController::reset()
    {

    }

    void NetlistSimulatorController::simulate(u64 picoseconds)
    {

    }
    /*
    void NetlistSimulatorController::setClock(const Net *n, int period, int start)
    {
        mClkNet = n;

        // TODO: clock set unify
        // mSimulator->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, 2000);
        mWaveDataList.addOrReplace(wd);
    }
*/

    void NetlistSimulatorController::handleSelectGates()
    {

        /*
        mSimulateGates = gsd.selectedGates();
        initSimulator();
        */
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
  //          mWaveDataList.addOrReplace(wdCopy);
        }
    }
        */
}    // namespace hal
