#include "vcd_viewer/vcd_viewer.h"

#include "vcd_viewer/wave_widget.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_engine.h"

#include "netlist_simulator_controller/vcd_serializer.h"
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
#include "netlist_simulator/netlist_simulator.h"
#include "netlist_simulator_controller/simulation_input.h"

namespace hal
{

    ContentWidget* VcdViewerFactory::contentFactory() const
    {
        return new VcdViewer;
    }

    VcdViewer::VcdViewer(QWidget *parent)
        : ContentWidget("VcdViewer",parent),
          mVisualizeNetState(false)
    {
        mCreateControlAction = new QAction(this);
        mSimulSettingsAction = new QAction(this);
        mOpenInputfileAction = new QAction(this);
        mRunSimulationAction = new QAction(this);

        mCreateControlAction->setIcon(gui_utility::getStyledSvgIcon("all->#808080",":/icons/plus"));
        mSimulSettingsAction->setIcon(gui_utility::getStyledSvgIcon("all->#808080",":/icons/preferences"));
        mOpenInputfileAction->setIcon(gui_utility::getStyledSvgIcon("all->#FFFFFF",":/icons/folder"));
        mRunSimulationAction->setIcon(gui_utility::getStyledSvgIcon("all->#20FF80",":/icons/run"));

        mCreateControlAction->setToolTip("Create simulation controller");
        mSimulSettingsAction->setToolTip("Simulation settings");
        mOpenInputfileAction->setToolTip("Open input file");
        mRunSimulationAction->setToolTip("Run Simulation");

        connect(mCreateControlAction, &QAction::triggered, this, &VcdViewer::handleCreateControl);
        connect(mSimulSettingsAction, &QAction::triggered, this, &VcdViewer::handleSimulSettings);
        connect(mOpenInputfileAction, &QAction::triggered, this, &VcdViewer::handleOpenInputFile);
        connect(mRunSimulationAction, &QAction::triggered, this, &VcdViewer::handleRunSimulation);

        mTabWidget = new QTabWidget(this);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        connect(mTabWidget,&QTabWidget::tabCloseRequested,this,&VcdViewer::handleTabClosed);
        mContentLayout->addWidget(mTabWidget);
//        mWaveWidget = new WaveWidget(this);
//        mWaveWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//        mContentLayout->addWidget(mWaveWidget);
        mStatusBar = new QStatusBar(this);
        mStatusBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mContentLayout->addWidget(mStatusBar);
        if (gSelectionRelay)
            connect(gSelectionRelay,&SelectionRelay::selectionChanged,this,&VcdViewer::handleSelectionChanged);

        NetlistSimulatorControllerMap* nscm = NetlistSimulatorControllerMap::instance();
        connect(nscm, &NetlistSimulatorControllerMap::controllerAdded, this, &VcdViewer::handleControllerAdded);
        connect(nscm, &NetlistSimulatorControllerMap::controllerRemoved, this, &VcdViewer::handleControllerRemoved);
        displayStatusMessage();
    }

    void VcdViewer::displayStatusMessage(const QString& msg)
    {
        if (msg.isEmpty())
        {
            if (mTabWidget->count())
            {
                WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->currentWidget());
                mStatusBar->showMessage("Setup simulation controller " + ww->controller()->name());
            }
            else
                mStatusBar->showMessage("Create new simulation controller");
        }
        else
            mStatusBar->showMessage(msg);
    }

    void VcdViewer::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mCreateControlAction);
        toolbar->addAction(mSimulSettingsAction);
        toolbar->addAction(mOpenInputfileAction);
        toolbar->addAction(mRunSimulationAction);

    }

    void VcdViewer::handleTabClosed(int inx)
    {
        WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
        if (!ww->triggerClose())
            log_warning(ww->controller()->get_name(), "Cannot close tab for externally owned controller.");
    }

    void VcdViewer::handleCreateControl()
    {
        NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
        if (!ctrlPlug)
        {
            log_warning("vcd_viewer", "Plugin 'netlist_simulator_controller' not found");
            return;
        }
        std::unique_ptr<NetlistSimulatorController> ctrlRef = ctrlPlug->create_simulator_controller();
        u32 ctrlId = ctrlRef.get()->get_id();
        for (int inx=0; inx<mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            qDebug() << "search controller" << ctrlId << ww->controllerId();
            if (ctrlId == ww->controllerId())
            {
                ww->takeOwnership(ctrlRef);
                break;
            }
        }
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
  // TODO       act->setEnabled(mState==SimulationClockSet);
         settingMenu->addAction(act);
         settingMenu->addSeparator();
         QMenu* engineMenu = settingMenu->addMenu("Select engine ...");
         QActionGroup* engineGroup = new QActionGroup(this);
         engineGroup->setExclusive(true);
         for (SimulationEngineFactory* sef : *SimulationEngineFactories::instance())
         {
             act = new QAction(QString::fromStdString(sef->name()), engineMenu);
             act->setCheckable(true);
             engineMenu->addAction(act);
             engineGroup->addAction(act);
         }
         settingMenu->addSeparator();
         act = new QAction("Visualize net state by color", settingMenu);
         act->setCheckable(true);
         act->setChecked(mVisualizeNetState);
         connect (act, &QAction::triggered, this, &VcdViewer::setVisualizeNetState);
         settingMenu->addAction(act);

         settingMenu->exec(mapToGlobal(QPoint(10,3)));
    }

    void VcdViewer::setVisualizeNetState(bool state)
    {
        if (state == mVisualizeNetState) return;
        mVisualizeNetState = state;
        for (int inx=0; inx < mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            ww->setVisualizeNetState(mVisualizeNetState, inx==mTabWidget->currentIndex());
        }
    }

    void VcdViewer::handleControllerAdded(u32 controllerId)
    {
        NetlistSimulatorController* nsc = NetlistSimulatorControllerMap::instance()->controller(controllerId);
        if (!nsc) return;
        WaveWidget* ww = new WaveWidget(nsc, mTabWidget);
        mTabWidget->addTab(ww,nsc->name());
        displayStatusMessage();
    }

    void VcdViewer::handleControllerRemoved(u32 controllerId)
    {
        for (int inx=0; inx<mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            if (ww->controllerId() == controllerId)
            {
                mTabWidget->removeTab(inx);
                ww->deleteLater();
            }
        }
        displayStatusMessage();
    }

    void VcdViewer::handleOpenInputFile()
    {
        if (!mTabWidget->count()) return;
        WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->currentWidget());
        if (!ww) return;
        QString filename =
                QFileDialog::getOpenFileName(this, "Load input wave file", ".", ("VCD Files (*.vcd)") );
        if (filename.isEmpty()) return;
        ww->controller()->parse_vcd(filename.toStdString());
    }

    void VcdViewer::handleRunSimulation()
    {
        mResultMap.clear();

        qDebug() << "results" << mResultMap.size();
    }

    void VcdViewer::handleClockSet()
    {
        WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->currentWidget());
        if (!ww) return;

        QList<const Net*> inputNets;
        for (const Net* n : ww->controller()->get_input_nets())
            inputNets.append(n);
        if (inputNets.isEmpty()) return;

        ClockSetDialog csd(inputNets, this);
        if (csd.exec() != QDialog::Accepted) return;

        int period = csd.period();
        if (period <= 0) return;

        const Net* clk = inputNets.at(csd.netIndex());
        ww->controller()->add_clock_period(clk,csd.duration(),csd.startValue()==0);

        // TODO : ww->update() ?
   }

    void VcdViewer::handleSelectGates()
    {
        WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->currentWidget());
        if (!ww) return;
        GateSelectionDialog gsd(this);
        if (gsd.exec() != QDialog::Accepted) return;

        ww->setGates(gsd.selectedGates());
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
// TODO             mWaveWidget->addOrReplaceWave(wdCopy);
        }
    }
}    // namespace hal
