#include "vcd_viewer/vcd_viewer.h"

#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_data.h"
#include "vcd_viewer/vcd_serializer.h"
#include "vcd_viewer/gate_selection_dialog.h"
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
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include "hal_core/plugin_system/plugin_manager.h"
#include "netlist_simulator/plugin_netlist_simulator.h"

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
        mContentLayout->addWidget(mWaveWidget);
        if (gSelectionRelay)
            connect(gSelectionRelay,&SelectionRelay::selectionChanged,this,&VcdViewer::handleSelectionChanged);
    }

    void VcdViewer::initSimulator()
    {
        NetlistSimulatorPlugin* simPlug = static_cast<NetlistSimulatorPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator"));
        if (!simPlug)
        {
            qDebug() << "Plugin 'netlist_simulator' not found";
            return;
        }
        qDebug() << "access to plugin" << simPlug->get_name().c_str() << simPlug->get_version().c_str();
        auto sim = simPlug->create_simulator();
        mOwner = std::move(sim);
        mSimulator = mOwner.get();
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
        for (Net* n : mInputNets)
        {
            WaveData* wd = new WaveData(n);
            wd->insert(0,0);
            mWaveWidget->addOrReplaceWave(wd);
        }
        mSimulator->set_iteration_timeout(1000);
        mState = SimulationClockSet;
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
        QMultiMap<int,QPair<Net*,SignalValue>> inputMap;
        for (Net* n : mSimulator->get_input_nets())
        {
            if (n==mClkNet) continue;
            const WaveData* wd = mWaveWidget->waveDataByNetId(n->get_id());
            for (auto it=wd->constBegin(); it != wd->constEnd(); ++it)
            {
                SignalValue sv;
                switch (it.value())
                {
                case -2: sv = SignalValue::Z; break;
                case -1: sv = SignalValue::X; break;
                case  0: sv = SignalValue::ZERO; break;
                case  1: sv = SignalValue::ONE; break;
                default: continue;
                }
                inputMap.insertMulti(it.key(),QPair<Net*,SignalValue>(n,sv));
            }
        }
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
            WaveData* wd = WaveData::simulationResultFactory(n, mSimulator);
            if (wd) mResultMap.insert(wd->id(),wd);
        }

        /*
        mSimulator->generate_vcd("result.vcd",0,t);

        VcdSerializer reader(this);
        for (WaveData* wd : reader.deserialize("result.vcd"))
            mResults.insert(wd->name(),wd);
            */
        qDebug() << "results" << mResultMap.size();
        mState = SimulationShowResults;
    }

    void VcdViewer::handleClockSet()
    {
        ClockSetDialog csd(mInputNets, this);
        if (csd.exec() != QDialog::Accepted) return;

        int period = csd.period();
        if (period <= 0) return;

        int start = csd.startValue();
        mClkNet = mInputNets.at(csd.netIndex());

        mSimulator->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, 2000);
        mWaveWidget->addOrReplaceWave(wd);
        mState = SimulationInputGenerate;
    }

    void VcdViewer::setClock(Net *n, int period, int start)
    {
        mClkNet = n;
        mSimulator->add_clock_period(mClkNet,period,start==0);
        WaveData* wd = WaveData::clockFactory(mClkNet, start, period, 2000);
        mWaveWidget->addOrReplaceWave(wd);
        mState = SimulationInputGenerate;
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
