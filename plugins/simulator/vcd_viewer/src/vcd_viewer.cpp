#include "vcd_viewer/vcd_viewer.h"

#include "vcd_viewer/wave_widget.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"

#include "vcd_viewer/vcd_serializer.h"
#include "vcd_viewer/gate_selection_dialog.h"
#include "vcd_viewer/plugin_vcd_viewer.h"
#include "vcd_viewer/clock_set_dialog.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_version.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"

#include <QFile>
#include <QDate>
#include <QDebug>
#include <QColor>
#include <QFileDialog>
#include <QStatusBar>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include "hal_core/plugin_system/plugin_manager.h"
#include "netlist_simulator/plugin_netlist_simulator.h"
#include "netlist_simulator_controller/simulation_input.h"

namespace hal
{

    ContentWidget* VcdViewerFactory::contentFactory() const
    {
        return new VcdViewer;
    }

    VcdViewer::VcdViewer(QWidget *parent)
        : ContentWidget("VcdViewer",parent), mState(SimulationSelectGates), mClkNet(nullptr)
    {
        mSimulSettingsAction = new QAction(this);
        mOpenInputfileAction = new QAction(this);
        mRunSimulationAction = new QAction(this);


        mSimulSettingsAction->setIcon(gui_utility::getStyledSvgIcon("all->#808080",":/icons/preferences"));
        mOpenInputfileAction->setIcon(gui_utility::getStyledSvgIcon("all->#FFFFFF",":/icons/folder"));
        mRunSimulationAction->setIcon(gui_utility::getStyledSvgIcon("all->#20FF80",":/icons/run"));

        mSimulSettingsAction->setToolTip("Simulation settings");
        mOpenInputfileAction->setToolTip("Open input file");
        mRunSimulationAction->setToolTip("Run Simulation");

        connect(mSimulSettingsAction, &QAction::triggered, this, &VcdViewer::handleSimulSettings);
        connect(mOpenInputfileAction, &QAction::triggered, this, &VcdViewer::handleOpenInputFile);
        connect(mRunSimulationAction, &QAction::triggered, this, &VcdViewer::handleRunSimulation);

        mWaveWidget = new WaveWidget(this);
        mWaveWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        mContentLayout->addWidget(mWaveWidget);
        mStatusBar = new QStatusBar(this);
        mStatusBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mContentLayout->addWidget(mStatusBar);
        if (gSelectionRelay)
            connect(gSelectionRelay,&SelectionRelay::selectionChanged,this,&VcdViewer::handleSelectionChanged);
        setState(SimulationSelectGates);
    }

    void VcdViewer::setState(SimulationState stat)
    {
        mState = stat;
        switch (mState)
        {
        case SimulationSelectGates:   mStatusBar->showMessage("Select gates for simulation");                   break;
        case SimulationClockSet:      mStatusBar->showMessage("Select and set clock");                          break;
        case SimulationInputGenerate: mStatusBar->showMessage("Generate input and start simulation when done"); break;
        case SimulationShowResults:   mStatusBar->showMessage("Select wires in graph to show results");         break;
        }
    }

    void VcdViewer::initSimulator()
    {
        NetlistSimulatorControllerPlugin* simControlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
        if (!simControlPlug)
        {
            qDebug() << "Plugin 'netlist_simulator_controller' not found";
            return;
        }
        qDebug() << "access to plugin" << simControlPlug->get_name().c_str() << simControlPlug->get_version().c_str();

        mController = simControlPlug->create_simulator_controller();

        /*
        mSimulator = simP->get_shared_simulator("vcd_viewer");
        if (!mSimulator)
        {
            qDebug() << "Cannot create new simulator";
            return;
        }
        */
        mController->reset();
        qDebug() << "sim has gates " << mController->get_gates().size();
        if (!gNetlist)
        {
            qDebug() << "No netlist loaded";
            return;
        }
        qDebug() << "net has gates " << gNetlist->get_gates().size();
        mController->add_gates(mSimulateGates);

        // TODO : WaveData from WaveDataList

        mClkNet = nullptr;
        mInputNets = mController->input()->get_input_nets();
        for (const Net* n : mInputNets)
        {
            WaveData* wd = new WaveData(n);
            wd->insert(0,0);
            mWaveWidget->addOrReplaceWave(wd);
        }

        // mController->set_iteration_timeout(1000);
        setState(SimulationClockSet);
    }

    void VcdViewer::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mSimulSettingsAction);
        toolbar->addAction(mOpenInputfileAction);
        toolbar->addAction(mRunSimulationAction);

    }

    void VcdViewer::handleSimulSettings()
    {
         QMenu* settingMenu = new QMenu(this);
         QAction* act;
         act = new QAction("Select gates for simulation", settingMenu);
         connect(act, &QAction::triggered, this, &VcdViewer::handleSelectGates);
         settingMenu->addAction(act);
         act = new QAction("Select clock net", settingMenu);
         connect(act, &QAction::triggered, this, &VcdViewer::handleClockSet);
         act->setEnabled(mState==SimulationClockSet);
         settingMenu->addAction(act);
         settingMenu->addSeparator();
         act = new QAction("Visualize net state by color", settingMenu);
         act->setCheckable(true);
         act->setChecked(mWaveWidget->isVisulizeNetState());
         connect (act, &QAction::triggered, mWaveWidget, &WaveWidget::setVisualizeNetState);
         settingMenu->addAction(act);

         settingMenu->exec(mapToGlobal(QPoint(10,3)));
    }

    void VcdViewer::handleOpenInputFile()
    {
        QString filename =
                QFileDialog::getOpenFileName(this, "Load input wave file", ".", ("VCD Files (*.vcd)") );
        if (filename.isEmpty()) return;
        VcdSerializer reader;
        for (WaveData* wd : reader.deserialize(filename))
            mWaveWidget->addOrReplaceWave(wd);
    }

    void VcdViewer::handleRunSimulation()
    {
        mResultMap.clear();
        if (!mClkNet)
        {
            qDebug() << "no clock found";
            return;
        }
        if (mState != SimulationInputGenerate)
        {
            qDebug() << "wrong state" << mState;
            return;
        }
        QMultiMap<int,QPair<const Net*, BooleanFunction::Value>> inputMap;
        for (const Net* n : mController->input()->get_input_nets())
        {
            if (n==mClkNet) continue;
            const WaveData* wd = mWaveWidget->waveDataByNetId(n->get_id());
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
        /*
         * TODO start simulation
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

        for (Net* n : gNetlist->get_nets())
        {
             TODO get data from engine
            WaveData* wd = WaveData::simulationResultFactory(n, mSimulator.get());
            if (wd) mResultMap.insert(wd->id(),wd);

        }


        mSimulator->generate_vcd("result.vcd",0,t);

        VcdSerializer reader(this);
        for (WaveData* wd : reader.deserialize("result.vcd"))
            mResults.insert(wd->name(),wd);
            */
        qDebug() << "results" << mResultMap.size();
        setState(SimulationShowResults);
    }

    void VcdViewer::handleClockSet()
    {
        ClockSetDialog csd(mInputNets, this);
        if (csd.exec() != QDialog::Accepted) return;

        int period = csd.period();
        if (period <= 0) return;

        int start = csd.startValue();
        mClkNet = mInputNets.at(csd.netIndex());

        mDuration = csd.duration();
        mController->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, mDuration);
        mWaveWidget->addOrReplaceWave(wd);
        setState(SimulationInputGenerate);
    }

    void VcdViewer::setClock(const Net *n, int period, int start)
    {
        mClkNet = n;
        mController->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, 2000);
        mWaveWidget->addOrReplaceWave(wd);
        setState(SimulationInputGenerate);
    }

    void VcdViewer::handleSelectGates()
    {
        GateSelectionDialog gsd(this);
        if (gsd.exec() != QDialog::Accepted) return;

        mSimulateGates = gsd.selectedGates();
        initSimulator();
    }

    void VcdViewer::handleSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);
        for (u32 nid : gSelectionRelay->selectedNetsList())
        {
            Net* n = gNetlist->get_net_by_id(nid);
            if (!n) continue;
            const WaveData* wd = mResultMap.value(n->get_id());
            if (!wd) continue;
            WaveData* wdCopy = new WaveData(*wd);
            mWaveWidget->addOrReplaceWave(wdCopy);
        }
    }
}    // namespace hal
