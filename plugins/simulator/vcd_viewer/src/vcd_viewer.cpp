#include "vcd_viewer/vcd_viewer.h"

#include "vcd_viewer/wave_widget.h"
#include "netlist_simulator_controller/simulation_process.h"
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
#include <QColor>
#include <QFileDialog>
#include <QStatusBar>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QScreen>
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
          mVisualizeNetState(false), mCurrentWaveWidget(nullptr)
    {
        LogManager::get_instance().add_channel(std::string("vcd_viewer"), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        mCreateControlAction = new QAction(this);
        mSimulSettingsAction = new QAction(this);
        mOpenInputfileAction = new QAction(this);
        mSaveWaveformsAction = new QAction(this);
        mRunSimulationAction = new QAction(this);
        mAddResultWaveAction = new QAction(this);

        mCreateControlAction->setIcon(gui_utility::getStyledSvgIcon("all->#FFFFFF",":/icons/plus"));
        mAddResultWaveAction->setIcon(QIcon(":/icons/add_waveform"));

        mCreateControlAction->setToolTip("Create simulation controller");
        mSimulSettingsAction->setToolTip("Simulation settings");
        mOpenInputfileAction->setToolTip("Open input file");
        mSaveWaveformsAction->setToolTip("Save waveform data to file");
        mRunSimulationAction->setToolTip("Run simulation");
        mAddResultWaveAction->setToolTip("Add waveform net");

        connect(mCreateControlAction, &QAction::triggered, this, &VcdViewer::handleCreateControl);
        connect(mSimulSettingsAction, &QAction::triggered, this, &VcdViewer::handleSimulSettings);
        connect(mOpenInputfileAction, &QAction::triggered, this, &VcdViewer::handleOpenInputFile);
        connect(mSaveWaveformsAction, &QAction::triggered, this, &VcdViewer::handleSaveWaveforms);
        connect(mRunSimulationAction, &QAction::triggered, this, &VcdViewer::handleRunSimulation);
        connect(mAddResultWaveAction, &QAction::triggered, this, &VcdViewer::handleAddResultWave);

        mTabWidget = new QTabWidget(this);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        connect(mTabWidget,&QTabWidget::tabCloseRequested,this,&VcdViewer::handleTabClosed);
        connect(mTabWidget,&QTabWidget::currentChanged,this,&VcdViewer::currentTabChanged);
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
        currentStateChanged(NetlistSimulatorController::NoGatesSelected);
    }

    VcdViewer::~VcdViewer()
    {
        closeEvent(nullptr);
    }

    void VcdViewer::closeEvent(QCloseEvent*event)
    {
        Q_UNUSED(event);
        NetlistSimulatorControllerMap* nscm = NetlistSimulatorControllerMap::instance();
        disconnect(nscm, &NetlistSimulatorControllerMap::controllerAdded, this, &VcdViewer::handleControllerAdded);
        disconnect(nscm, &NetlistSimulatorControllerMap::controllerRemoved, this, &VcdViewer::handleControllerRemoved);
    }


    void VcdViewer::currentTabChanged(int inx)
    {
        if (inx >= 0 && mCurrentWaveWidget == mTabWidget->widget(inx)) return;
        if (mCurrentWaveWidget)
            disconnect(mCurrentWaveWidget,&WaveWidget::stateChanged,this,&VcdViewer::currentStateChanged);
        if (inx < 0)
        {
            mCurrentWaveWidget = nullptr;
            currentStateChanged(NetlistSimulatorController::NoGatesSelected);
            return;
        }
        mCurrentWaveWidget =  static_cast<WaveWidget*>(mTabWidget->widget(inx));
        currentStateChanged(mCurrentWaveWidget->state());
        connect(mCurrentWaveWidget,&WaveWidget::stateChanged,this,&VcdViewer::currentStateChanged);
    }

    void VcdViewer::handleParseComplete()
    {
        if (!mCurrentWaveWidget) return;
        mAddResultWaveAction->setEnabled(mCurrentWaveWidget->canImportWires());
    }

    void VcdViewer::currentStateChanged(NetlistSimulatorController::SimulationState state)
    {
        if (!mCurrentWaveWidget || state == NetlistSimulatorController::SimulationRun)
        {
            mSimulSettingsAction->setDisabled(true);
            mOpenInputfileAction->setDisabled(true);
            mSaveWaveformsAction->setDisabled(true);
            mRunSimulationAction->setDisabled(true);
            mAddResultWaveAction->setDisabled(true);
        }
        else
        {
            mSimulSettingsAction->setEnabled(true);
            mOpenInputfileAction->setEnabled(state == NetlistSimulatorController::ParameterSetup
                                             || state == NetlistSimulatorController::ParameterReady);
            mSaveWaveformsAction->setEnabled(state != NetlistSimulatorController::NoGatesSelected);
            mRunSimulationAction->setEnabled(state == NetlistSimulatorController::ParameterReady);
       }
        mSimulSettingsAction->setIcon(gui_utility::getStyledSvgIcon(mSimulSettingsAction->isEnabled() ? "all->#FFFFFF" : "all->#808080",":/icons/preferences"));
        mOpenInputfileAction->setIcon(gui_utility::getStyledSvgIcon(mOpenInputfileAction->isEnabled() ? "all->#3192C5" : "all->#808080",":/icons/folder"));
        mSaveWaveformsAction->setIcon(gui_utility::getStyledSvgIcon(mSaveWaveformsAction->isEnabled() ? "all->#3192C5" : "all->#808080",":/icons/save"));
        mRunSimulationAction->setIcon(gui_utility::getStyledSvgIcon(mRunSimulationAction->isEnabled() ? "all->#20FF80" : "all->#808080",":/icons/run"));

        if (!mCurrentWaveWidget)
            displayStatusMessage();
        else
        {
            mAddResultWaveAction->setEnabled(mCurrentWaveWidget->canImportWires());
            switch (state) {
            case NetlistSimulatorController::NoGatesSelected: displayStatusMessage("Select gates to create (partial) netlist for simulation");       break;
            case NetlistSimulatorController::ParameterSetup:  displayStatusMessage("Setup parameter for simulation");                                break;
            case NetlistSimulatorController::ParameterReady:  displayStatusMessage("Continue parameter setup or start simulation");                                break;
            case NetlistSimulatorController::SimulationRun:   displayStatusMessage("Simulation engine running, please wait ...");                    break;
            case NetlistSimulatorController::ShowResults:     displayStatusMessage("Simulation successful, add waveform data to visualize results"); break;
            case NetlistSimulatorController::EngineFailed:    displayStatusMessage("Simulation engine failed");                                      break;
            }
        }
    }

    void VcdViewer::displayStatusMessage(const QString& msg)
    {
        if (msg.isEmpty())
        {
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
        toolbar->addAction(mSaveWaveformsAction);
        toolbar->addAction(mRunSimulationAction);
        toolbar->addAction(mAddResultWaveAction);
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
         // TODO : enable/disable according to state of current WaveWidget/Controller
         settingMenu->addAction(act);

         settingMenu->addSeparator();
         QMenu* engineMenu = settingMenu->addMenu("Select engine ...");
         QActionGroup* engineGroup = new QActionGroup(this);
         engineGroup->setExclusive(true);
         for (SimulationEngineFactory* sef : *SimulationEngineFactories::instance())
         {
             act = new QAction(QString::fromStdString(sef->name()), engineMenu);
             act->setCheckable(true);
             connect(act,&QAction::triggered,this,&VcdViewer::handleEngineSelected);
             engineMenu->addAction(act);
             engineGroup->addAction(act);
         }

         act = new QAction("Set engine properties");
         connect(act, &QAction::triggered, this, &VcdViewer::handleSetEngineProperties);
         settingMenu->addAction(act);

         act = new QAction("Show output of engine");
         connect(act, &QAction::triggered, this, &VcdViewer::handleShowEngineOutput);
         settingMenu->addAction(act);

         settingMenu->addSeparator();
         act = new QAction("Refresh net names", settingMenu);
         connect(act, &QAction::triggered, this, &VcdViewer::handleRefreshNetNames);
         settingMenu->addAction(act);

         act = new QAction("Visualize net state by color", settingMenu);
         act->setCheckable(true);
         act->setChecked(mVisualizeNetState);
         connect (act, &QAction::triggered, this, &VcdViewer::setVisualizeNetState);
         settingMenu->addAction(act);

         act = new QAction("Net groups: display text value in graph", settingMenu);
         act->setCheckable(true);
         act->setChecked(WaveItem::sValuesAsText);
         connect (act, &QAction::triggered, this, &VcdViewer::setGroupValuesAsText);
         settingMenu->addAction(act);

         settingMenu->exec(mapToGlobal(QPoint(10,3)));
    }

    void VcdViewer::handleSetEngineProperties()
    {
        if (!mCurrentWaveWidget) return;
        SimulationEngine* eng = mCurrentWaveWidget->controller()->get_simulation_engine();
        if (!eng) return;

    }

    void VcdViewer::handleShowEngineOutput()
    {
        if (!mCurrentWaveWidget) return;
        QString fname = QDir(QString::fromStdString(mCurrentWaveWidget->controller()->get_working_directory())).absoluteFilePath(SimulationProcessLog::sLogFilename);
        QFile log(fname);
        if (!log.open(QIODevice::ReadOnly)) return;
        QTextEdit* browser = new QTextEdit;
        browser->setReadOnly(true);
        browser->setHtml(QString::fromUtf8(log.readAll()));
        QSize scrSize = QGuiApplication::primaryScreen()->size();
        browser->setGeometry(scrSize.width()/10,scrSize.height()/10,scrSize.width()*4/5,scrSize.height()*4/5);
        browser->setWindowTitle("Simulation engine log <" + fname + ">");
        browser->show();
    }

    void VcdViewer::handleEngineSelected(bool checked)
    {
        if (!checked || !mCurrentWaveWidget) return;
        const QAction* act = static_cast<const QAction*>(sender());
        if (!act) return;
        mCurrentWaveWidget->createEngine(act->text());
    }

    void VcdViewer::handleRefreshNetNames()
    {
        for (int inx=0; inx<mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            ww->refreshNetNames();
        }
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

    void VcdViewer::setGroupValuesAsText(bool state)
    {
        if (state == WaveItem::sValuesAsText) return;
        WaveItem::sValuesAsText = state;
        for (int inx=0; inx < mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            ww->update();
        }
    }

    void VcdViewer::handleControllerAdded(u32 controllerId)
    {
        NetlistSimulatorController* nsc = NetlistSimulatorControllerMap::instance()->controller(controllerId);
        if (!nsc) return;
        WaveWidget* ww = new WaveWidget(nsc, mTabWidget);
        mTabWidget->addTab(ww,nsc->name());
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
    }

    void VcdViewer::handleOpenInputFile()
    {
        if (!mCurrentWaveWidget) return;

        QString filename =
                QFileDialog::getOpenFileName(this, "Load input wave file", ".", ("VCD files (*.vcd);; CSV files (*.csv)") );
        if (filename.isEmpty()) return;
        if (filename.toLower().endsWith(".vcd"))
            mCurrentWaveWidget->controller()->import_vcd(filename.toStdString(),NetlistSimulatorController::GlobalInputs);
        else if (filename.toLower().endsWith(".csv"))
            mCurrentWaveWidget->controller()->import_csv(filename.toStdString(),NetlistSimulatorController::GlobalInputs);
        else
            log_warning(mCurrentWaveWidget->controller()->get_name(), "Unknown extension, cannot parse file '{}'.", filename.toStdString());
    }

    void VcdViewer::handleSaveWaveforms()
    {
        if (!mCurrentWaveWidget) return;

        QString filename =
                QFileDialog::getSaveFileName(this, "Save waveform data to file", ".", ("VCD Files (*.vcd)") );
        if (filename.isEmpty()) return;
        mCurrentWaveWidget->controller()->generate_vcd(filename.toStdString());
    }

    void VcdViewer::handleRunSimulation()
    {
        if (!mCurrentWaveWidget) return;
        connect(mCurrentWaveWidget->controller(),&NetlistSimulatorController::engineFinished,mCurrentWaveWidget,&WaveWidget::handleEngineFinished);
        mCurrentWaveWidget->controller()->run_simulation();
    }

    void VcdViewer::handleAddResultWave()
    {
        if (!mCurrentWaveWidget) return;
        mCurrentWaveWidget->addResults();
        mAddResultWaveAction->setEnabled(mCurrentWaveWidget->canImportWires());
    }

    void VcdViewer::handleClockSet()
    {
        if (!mCurrentWaveWidget) return;

        QList<const Net*> inputNets;
        for (const Net* n : mCurrentWaveWidget->controller()->get_input_nets())
            inputNets.append(n);
        if (inputNets.isEmpty()) return;

        ClockSetDialog csd(inputNets, this);
        if (csd.exec() != QDialog::Accepted) return;

        if (csd.dontUseClock())
            mCurrentWaveWidget->controller()->set_no_clock_used();
        else
        {
            int period = csd.period();
            if (period <= 0) return;

            const Net* clk = inputNets.at(csd.netIndex());
            mCurrentWaveWidget->controller()->add_clock_period(clk,period,csd.startValue()==0,csd.duration());
        }
    }

    void VcdViewer::handleSelectGates()
    {
        if (!mCurrentWaveWidget) return;
        GateSelectionDialog gsd(this);
        if (gsd.exec() != QDialog::Accepted) return;

        mCurrentWaveWidget->setGates(gsd.selectedGates());
    }

    void VcdViewer::handleSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);
        /*
        for (u32 nid : gSelectionRelay->selectedNetsList())
        {
            Net* n = gNetlist->get_net_by_id(nid);
            if (!n) continue;
            const WaveData* wd = mResultMap.value(n->get_id());
            if (!wd) continue;
            WaveData* wdCopy = new WaveData(*wd);
// TODO             mWaveWidget->addOrReplaceWave(wdCopy);
        }
        */
    }
}    // namespace hal
