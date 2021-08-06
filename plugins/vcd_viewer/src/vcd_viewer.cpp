#include "vcd_viewer/vcd_viewer.h"

#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_data.h"
#include "vcd_viewer/vcd_serializer.h"
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
        : ContentWidget("VcdViewer",parent), mClkNet(nullptr)
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
        initSimulator();
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
        qDebug() << "sim has gates " << mSimulator->get_gates().size();
        if (!gNetlist)
        {
            qDebug() << "No netlist loaded";
            return;
        }
        qDebug() << "net has gates " << gNetlist->get_gates().size();
        mSimulator->add_gates(gNetlist->get_gates());
        for (Net* n : gNetlist->get_nets())
            if (n->get_name() == "CLK")
            {
                mClkNet = n;
                break;
            }
        if (!mClkNet)
        {
            qDebug() << "No clock found";
            return;
        }

        for (Net* n : mSimulator->get_input_nets())
        {
            if (n==mClkNet) continue;
            WaveData* wd = new WaveData(QString("%1[%2]").arg(QString::fromStdString(n->get_name())).arg(n->get_id()));
            wd->insert(0,0);
            mWaveWidget->addOrReplaceWave(wd);
            qDebug() << "input net" << n->get_id() << n->get_name().c_str();
        }
        mSimulator->add_clock_frequency(mClkNet,1000000);
        mSimulator->set_iteration_timeout(1000);
    }

    void VcdViewer::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mSimulSettingsAction);
        toolbar->addAction(mOpenInputfileAction);
        toolbar->addAction(mRunSimulationAction);
    }

    void VcdViewer::handleSimulSettings()
    {
         qDebug() << "simulation settings";
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
        mResults.clear();
        if (!mClkNet)
        {
            qDebug() << "no clock found";
            return;
        }
        QMultiMap<int,QPair<Net*,SignalValue>> inputMap;
        for (Net* n : mSimulator->get_input_nets())
        {
            if (n==mClkNet) continue;
            const WaveData* wd = mWaveWidget->waveDataByName(QString("%1[%2]").arg(QString::fromStdString(n->get_name())).arg(n->get_id()));
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
        mSimulator->generate_vcd("result.vcd",0,t);

        VcdSerializer reader(this);
        for (WaveData* wd : reader.deserialize("result.vcd"))
            mResults.insert(wd->name(),wd);
        qDebug() << "results" << mResults.size();
    }

    void VcdViewer::handleSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);
        for (u32 nid : gSelectionRelay->selectedNetsList())
        {
            Net* n = gNetlist->get_net_by_id(nid);
            if (!n) continue;
            QString name = QString::fromStdString(n->get_name());
            const WaveData* wd = mResults.value(name);
            if (!wd) continue;
            name += QString("[%1]").arg(n->get_id());
            WaveData* wdCopy = new WaveData(name, *wd);
            mWaveWidget->addOrReplaceWave(wdCopy);
        }
    }
}    // namespace hal
