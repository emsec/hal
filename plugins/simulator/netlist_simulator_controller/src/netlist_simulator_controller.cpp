#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_input.h"

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
        : QObject(parent), mState(NoGatesSelected), mClkNet(nullptr),
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
        mResultMap.clear();
        if (!mClkNet)
        {
            qDebug() << "no clock found";
            return;
        }
        if (mState != ParameterSetup)
        {
            qDebug() << "wrong state" << mState;
            return;
        }
        QMultiMap<int,QPair<const Net*, BooleanFunction::Value>> inputMap;
        for (const Net* n : mSimulationInput->get_input_nets())
        {
            if (n==mClkNet) continue;
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

        /* TODO trigger simulation
        int t=0;
        for (auto it = inputMap.begin(); it != inputMap.end(); ++it)
        {
            if (it.key() != t)
            {
                mSimulator->simulate(it.key() - t);
                t = it.key();
            }
            mSimulator->set_input(it.value().first,it.value().second);
        }

        */
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

    void NetlistSimulatorController::handleClockSet()
    {
        /*
        int period = csd.period();
        if (period <= 0) return;

        int start = csd.startValue();
        mClkNet = mInputNets.at(csd.netIndex());

        mDuration = csd.duration();
        mSimulator->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, mDuration);
        mWaveDataList.addOrReplace(wd);
        setState(SimulationInputGenerate);
        */
    }

    void NetlistSimulatorController::setClock(const Net *n, int period, int start)
    {
        mClkNet = n;

        // TODO: clock set unify
        // mSimulator->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, 2000);
        mWaveDataList.addOrReplace(wd);
    }

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
