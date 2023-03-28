#include "waveform_viewer/waveform_viewer.h"

#include "waveform_viewer/wave_widget.h"
#include "waveform_viewer/simulation_setting_dialog.h"
#include "netlist_simulator_controller/simulation_process.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_engine.h"

#include "waveform_viewer/wizard.h"

#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "waveform_viewer/gate_selection_dialog.h"
#include "waveform_viewer/clock_set_dialog.h"
#include "waveform_viewer/wave_graphics_canvas.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_manager.h"
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
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QInputDialog>
#include <QTabBar>
#include "hal_core/plugin_system/plugin_manager.h"

namespace hal
{

    ContentWidget* VcdViewerFactory::contentFactory() const
    {
        return new WaveformViewer;
    }

    void WaveformViewer::restoreFromProject()
    {
        NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
        if (!ctrlPlug || !ctrlPlug->sSimulatorSerializer)
        {
            return;
        }
        for (std::unique_ptr<NetlistSimulatorController>& ctrlRef : ctrlPlug->sSimulatorSerializer->restore())
        {
            if (!ctrlRef) continue;
            takeControllerOwnership(ctrlRef, false); // already created by restore()
        }
    }

    WaveformViewer::WaveformViewer(QWidget *parent)
        : ContentWidget("WaveformViewer",parent),
          mVisualizeNetState(false), mCurrentWaveWidget(nullptr)
    {
        LogManager::get_instance()->add_channel(std::string("waveform_viewer"), {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        mCreateControlAction = new QAction(this);
        mSimulSettingsAction = new QAction(this);
        mOpenInputfileAction = new QAction(this);
        mSaveWaveformsAction = new QAction(this);
        mRunSimulationAction = new QAction(this);
        mAddResultWaveAction = new QAction(this);
        mToggleMaxZoomAction = new QAction(this);
        mUndoZoomShiftAction = new QAction(this);

        mCreateControlAction->setIcon(gui_utility::getStyledSvgIcon("all->#FFFFFF",":/icons/plus"));
        mAddResultWaveAction->setIcon(QIcon(":/icons/add_waveform"));
        mToggleMaxZoomAction->setIcon(QIcon(":/icons/zoom_waveform"));
        mUndoZoomShiftAction->setIcon(gui_utility::getStyledSvgIcon("all->#FFFFFF",":/icons/undo2"));

        mCreateControlAction->setToolTip("Create simulation controller");
        mSimulSettingsAction->setToolTip("Simulation settings");
        mOpenInputfileAction->setToolTip("Open input file");
        mSaveWaveformsAction->setToolTip("Save waveform data to file");
        mRunSimulationAction->setToolTip("Run simulation");
        mAddResultWaveAction->setToolTip("Add waveform net");
        mToggleMaxZoomAction->setToolTip("Toggle max/min zoom");
        mUndoZoomShiftAction->setToolTip("Undo last zoom or horizontal scroll");

        connect(mCreateControlAction, &QAction::triggered, this, &WaveformViewer::handleCreateControl);
        connect(mSimulSettingsAction, &QAction::triggered, this, &WaveformViewer::handleSimulSettings);
        connect(mOpenInputfileAction, &QAction::triggered, this, &WaveformViewer::handleOpenInputFile);
        connect(mSaveWaveformsAction, &QAction::triggered, this, &WaveformViewer::handleSaveWaveforms);
        connect(mRunSimulationAction, &QAction::triggered, this, &WaveformViewer::handleRunSimulation);
        connect(mAddResultWaveAction, &QAction::triggered, this, &WaveformViewer::handleAddResultWave);
        connect(mToggleMaxZoomAction, &QAction::triggered, this, &WaveformViewer::handleToggleMaxZoom);
        connect(mUndoZoomShiftAction, &QAction::triggered, this, &WaveformViewer::handleUndoZoomShift);

        mTabWidget = new QTabWidget(this);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        connect(mTabWidget,&QTabWidget::tabCloseRequested,this,&WaveformViewer::handleTabClosed);
        connect(mTabWidget,&QTabWidget::currentChanged,this,&WaveformViewer::currentTabChanged);
        mContentLayout->addWidget(mTabWidget);
        mStatusBar = new QStatusBar(this);
        mProgress = new QProgressBar(mStatusBar);
        mProgress->setFormat("Import waveform data : %p%");
        mProgress->setTextVisible(true);
        mProgress->setRange(0,100);
        mStatusBar->addWidget(mProgress,100);
        mProgress->hide();
        mStatusBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mContentLayout->addWidget(mStatusBar);
        if (gSelectionRelay)
            connect(gSelectionRelay,&SelectionRelay::selectionChanged,this,&WaveformViewer::handleSelectionChanged);

        NetlistSimulatorControllerMap* nscm = NetlistSimulatorControllerMap::instance();
        connect(nscm, &NetlistSimulatorControllerMap::controllerAdded,
                this, &WaveformViewer::handleControllerAdded, Qt::QueuedConnection);
        connect(nscm, &NetlistSimulatorControllerMap::controllerRemoved,
                this, &WaveformViewer::handleControllerRemoved, Qt::QueuedConnection);
        currentStateChanged(NetlistSimulatorController::NoGatesSelected);
    }

    WaveformViewer::~WaveformViewer()
    {
        closeEvent(nullptr);
    }

    void WaveformViewer::closeEvent(QCloseEvent*event)
    {
        Q_UNUSED(event);
        NetlistSimulatorControllerMap* nscm = NetlistSimulatorControllerMap::instance();
        disconnect(nscm, &NetlistSimulatorControllerMap::controllerAdded, this, &WaveformViewer::handleControllerAdded);
        disconnect(nscm, &NetlistSimulatorControllerMap::controllerRemoved, this, &WaveformViewer::handleControllerRemoved);
    }

    void WaveformViewer::showProgress(int percent)
    {
        if (percent < 0)
        {
            mProgress->setValue(0);
            mProgress->hide();
            currentStateChanged(mCurrentWaveWidget->state());
        }
        else if (percent != mProgress->value())
        {
            mStatusBar->clearMessage();
            mProgress->show();
            mProgress->setValue(percent);
        }
    }

    void WaveformViewer::currentTabChanged(int inx)
    {
        if (inx >= 0 && mCurrentWaveWidget == mTabWidget->widget(inx)) return;
        if (mCurrentWaveWidget)
            disconnect(mCurrentWaveWidget,&WaveWidget::stateChanged,this,&WaveformViewer::currentStateChanged);
        if (inx < 0)
        {
            mCurrentWaveWidget = nullptr;
            currentStateChanged(NetlistSimulatorController::NoGatesSelected);
            return;
        }
        mCurrentWaveWidget =  static_cast<WaveWidget*>(mTabWidget->widget(inx));
        currentStateChanged(mCurrentWaveWidget->state());
        connect(mCurrentWaveWidget,&WaveWidget::stateChanged,this,&WaveformViewer::currentStateChanged);
    }

    void WaveformViewer::handleParseComplete()
    {
        if (!mCurrentWaveWidget) return;
        mAddResultWaveAction->setEnabled(mCurrentWaveWidget->canImportWires());
    }

    void WaveformViewer::testUndoEnable()
    {
        if (!mCurrentWaveWidget)
            mUndoZoomShiftAction->setDisabled(true);
        else
            mUndoZoomShiftAction->setEnabled(mCurrentWaveWidget->graphicCanvas()->canUndoZoom());
        mUndoZoomShiftAction->setIcon(gui_utility::getStyledSvgIcon(mUndoZoomShiftAction->isEnabled() ? "all->#FFFFFF" : "all->#808080",":/icons/undo2"));
    }

    void WaveformViewer::currentStateChanged(NetlistSimulatorController::SimulationState state)
    {
        mOpenInputfileAction->setEnabled(true);
        mRunSimulationAction->setEnabled(true);
        mOpenInputfileAction->setIcon(gui_utility::getStyledSvgIcon("all->#3192C5",":/icons/folder"));
        if (!mCurrentWaveWidget || state == NetlistSimulatorController::SimulationRun || mProgress->isVisible())
        {
            mSimulSettingsAction->setDisabled(true);
            mSaveWaveformsAction->setDisabled(true);

            // TODO: das ganze muss anders gestartet werden sonst fehlen einpar ptr
            mRunSimulationAction->setDisabled(true);
            mAddResultWaveAction->setDisabled(true);
            mToggleMaxZoomAction->setDisabled(true);
        }
        else

        {
            mSimulSettingsAction->setEnabled(true);
            mSaveWaveformsAction->setEnabled(state != NetlistSimulatorController::NoGatesSelected);

            //mRunSimulationAction->setEnabled(state == NetlistSimulatorController::ParameterReady);
            mToggleMaxZoomAction->setEnabled(!mCurrentWaveWidget->isEmpty());
       }
        mSimulSettingsAction->setIcon(gui_utility::getStyledSvgIcon(mSimulSettingsAction->isEnabled() ? "all->#FFFFFF" : "all->#808080",":/icons/preferences"));

        mSaveWaveformsAction->setIcon(gui_utility::getStyledSvgIcon(mSaveWaveformsAction->isEnabled() ? "all->#3192C5" : "all->#808080",":/icons/save"));
        mRunSimulationAction->setIcon(gui_utility::getStyledSvgIcon(mRunSimulationAction->isEnabled() ? "all->#20FF80" : "all->#808080",":/icons/run"));
        testUndoEnable();

        if (!mCurrentWaveWidget)
            displayStatusMessage();
        else
        {
            mAddResultWaveAction->setEnabled(mCurrentWaveWidget->canImportWires());
            switch (state) {
            case NetlistSimulatorController::NoGatesSelected: displayStatusMessage("Select gates to create (partial) netlist for simulation");       break;
            case NetlistSimulatorController::ParameterSetup:  displayStatusMessage("Setup parameter for simulation");                                break;
            case NetlistSimulatorController::ParameterReady:  displayStatusMessage("Continue parameter setup or start simulation");                  break;
            case NetlistSimulatorController::SimulationRun:   displayStatusMessage("Simulation engine running, please wait ...");                    break;
            case NetlistSimulatorController::ShowResults:     displayStatusMessage("Simulation successful, add waveform data to visualize results"); break;
            case NetlistSimulatorController::EngineFailed:    displayStatusMessage("Simulation engine failed");                                      break;
            }
        }
    }

    void WaveformViewer::displayStatusMessage(const QString& msg)
    {
        if (msg.isEmpty())
        {
            mStatusBar->showMessage("Create new simulation controller");
        }
        else
            mStatusBar->showMessage(msg);
    }

    void WaveformViewer::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mCreateControlAction);
        toolbar->addAction(mSimulSettingsAction);
        toolbar->addAction(mOpenInputfileAction);
        toolbar->addAction(mSaveWaveformsAction);
        toolbar->addAction(mRunSimulationAction);
        toolbar->addAction(mAddResultWaveAction);
        toolbar->addAction(mToggleMaxZoomAction);
        toolbar->addAction(mUndoZoomShiftAction);
        mRunSimulationAction->setEnabled(true);
    }

    void WaveformViewer::handleTabClosed(int inx)
    {
        WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
        if (!ww->triggerClose())
            log_warning(ww->controller()->get_name(), "Cannot close tab for externally owned controller.");
    }

    void WaveformViewer::handleCreateControl()
    {
        NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
        if (!ctrlPlug)
        {
            log_warning("waveform_viewer", "Plugin 'netlist_simulator_controller' not found");
            return;
        }
        std::unique_ptr<NetlistSimulatorController> ctrlRef = ctrlPlug->create_simulator_controller();
        if (ctrlRef)
            takeControllerOwnership(ctrlRef, false);
    }

    void WaveformViewer::handleSimulSettings()
    {
         QMenu* settingMenu = new QMenu(this);
         QAction* act;
         act = new QAction("Select gates for simulation", settingMenu);
         if (!mCurrentWaveWidget || mCurrentWaveWidget->state() != NetlistSimulatorController::NoGatesSelected)
             act->setDisabled(true);
         else
            connect(act, &QAction::triggered, this, &WaveformViewer::handleSelectGates);
         settingMenu->addAction(act);

         act = new QAction("Select clock net", settingMenu);
         connect(act, &QAction::triggered, this, &WaveformViewer::handleClockSet);
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
             connect(act,&QAction::triggered,this,&WaveformViewer::handleEngineSelected);
             engineMenu->addAction(act);
             engineGroup->addAction(act);
         }

         act = new QAction("Show output of engine");
         connect(act, &QAction::triggered, this, &WaveformViewer::handleShowEngineOutput);
         settingMenu->addAction(act);

         settingMenu->addSeparator();
         act = new QAction("Refresh net names", settingMenu);
         connect(act, &QAction::triggered, this, &WaveformViewer::handleRefreshNetNames);
         settingMenu->addAction(act);

         act = new QAction("Visualize net state by color", settingMenu);
         act->setCheckable(true);
         act->setChecked(mVisualizeNetState);
         connect (act, &QAction::triggered, this, &WaveformViewer::setVisualizeNetState);
         settingMenu->addAction(act);

         act = new QAction("Settings ...");
         connect(act, &QAction::triggered, this, &WaveformViewer::handleOpenSettingsDialog);
         settingMenu->addAction(act);

         settingMenu->exec(mapToGlobal(QPoint(10,3)));
    }

    void WaveformViewer::handleOpenSettingsDialog()
    {
        SimulationSettingDialog ssd(NetlistSimulatorControllerPlugin::sSimulationSettings,this);
        ssd.exec();
    }

    void WaveformViewer::handleShowEngineOutput()
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

    void WaveformViewer::handleEngineSelected(bool checked)
    {
        if (!checked || !mCurrentWaveWidget) return;
        const QAction* act = static_cast<const QAction*>(sender());
        if (!act) return;

        // check if verilator is installed
        if (act->text().toStdString() == "verilator")
        {
            std::string cmd = "which verilator";
            if (std::system(cmd.c_str()) != 0)
            {
                log_warning("simulation_plugin", "Verilator is not installed");
            }
        }


        mCurrentWaveWidget->createEngine(act->text());
    }

    void WaveformViewer::handleRefreshNetNames()
    {
        for (int inx=0; inx<mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            ww->refreshNetNames();
        }
    }

    void WaveformViewer::setVisualizeNetState(bool state)
    {
        if (state == mVisualizeNetState) return;
        mVisualizeNetState = state;
        for (int inx=0; inx < mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            ww->setVisualizeNetState(mVisualizeNetState, inx==mTabWidget->currentIndex());
        }
    }

    void WaveformViewer::handleControllerAdded(u32 controllerId)
    {
        // check whether controller already added
        for (int inx=0; inx<mTabWidget->count(); inx++)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(mTabWidget->widget(inx));
            if (controllerId == ww->controllerId()) return;
        }
        NetlistSimulatorController* nsc = NetlistSimulatorControllerMap::instance()->controller(controllerId);
        if (!nsc) return;
        WaveWidget* ww = new WaveWidget(nsc, mTabWidget);
        mTabWidget->addTab(ww,nsc->name());
        showCloseButton();
    }

    void WaveformViewer::handleControllerRemoved(u32 controllerId)
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

    void WaveformViewer::handleOpenInputFile()
    {
        QString filter = QString("Saved data (%1)").arg(NetlistSimulatorController::sPersistFile);
        if (mCurrentWaveWidget)
            filter += ";; VCD files (*.vcd);; CSV files (*.csv)";

        QString filename =
                QFileDialog::getOpenFileName(this, "Load input wave file", ".", filter);
        if (filename.isEmpty()) return;
        if (filename.endsWith(NetlistSimulatorController::sPersistFile))
        {
            NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
            if (ctrlPlug)
            {
                std::unique_ptr<NetlistSimulatorController> ctrlRef = ctrlPlug->restore_simulator_controller(gNetlist,filename.toStdString());
                takeControllerOwnership(ctrlRef, true);
            }
        }
        else if (mCurrentWaveWidget && mCurrentWaveWidget->controller()->can_import_data() && filename.toLower().endsWith(".vcd"))
            mCurrentWaveWidget->controller()->import_vcd(filename.toStdString(),NetlistSimulatorController::GlobalInputs);
        else if (mCurrentWaveWidget && mCurrentWaveWidget->controller()->can_import_data() && filename.toLower().endsWith(".csv"))
            mCurrentWaveWidget->controller()->import_csv(filename.toStdString(),NetlistSimulatorController::GlobalInputs);
        else if (mCurrentWaveWidget)
            log_warning(mCurrentWaveWidget->controller()->get_name(), "Cannot parse file '{}' (unknown extension ore wrong state).", filename.toStdString());
        else
            log_warning("simulation_plugin", "Unable to restore saved data from file '{}'.", filename.toStdString());
    }

    void WaveformViewer::showCloseButton()
    {
        for (int i=0; i<mTabWidget->count(); i++)
        {
            bool own = static_cast<const WaveWidget*>(mTabWidget->widget(i))->hasOwnership();
            for (int j=0; j<2; j++)
            {
                QWidget* closeBut = mTabWidget->tabBar()->tabButton(i,j?QTabBar::LeftSide:QTabBar::RightSide);
                if (closeBut && closeBut->metaObject()->className() == QByteArray("CloseButton"))
                {
                    if (own)
                    {
                        if (!closeBut->isVisible())
                            closeBut->show();
                    }
                    else
                    {
                        if (closeBut->isVisible())
                            closeBut->hide();
                    }
                }
            }
        }
    }

    void WaveformViewer::takeControllerOwnership(std::unique_ptr<NetlistSimulatorController> &ctrlRef, bool create)
    {
        NetlistSimulatorController* nsc = ctrlRef.get();

        WaveWidget* ww = nullptr;
        if (create)
        {
            ww = new WaveWidget(nsc, mTabWidget);
            mTabWidget->addTab(ww,nsc->name());
        }
        else
        {
            for (int inx=0; inx<mTabWidget->count(); inx++)
            {
                WaveWidget* wwTest = static_cast<WaveWidget*>(mTabWidget->widget(inx));
                if (ctrlRef.get()->get_id() == wwTest->controllerId())
                {
                    ww = wwTest;
                    break;
                }
            }

        }
        Q_ASSERT(ww);
        ww->takeOwnership(ctrlRef);
        showCloseButton();
    }

    void WaveformViewer::handleSaveWaveforms()
    {
        if (!mCurrentWaveWidget) return;

        QString filename =
                QFileDialog::getSaveFileName(this, "Save waveform data to file", ".", ("VCD Files (*.vcd)") );
        if (filename.isEmpty()) return;
        mCurrentWaveWidget->controller()->generate_vcd(filename.toStdString());
    }

    void WaveformViewer::handleRunSimulation()
    {
        Wizard wizard(this);
        wizard.exec();
        //if (!mCurrentWaveWidget) return;
        //connect(mCurrentWaveWidget->controller(),&NetlistSimulatorController::engineFinished,mCurrentWaveWidget,&WaveWidget::handleEngineFinished);
        //mCurrentWaveWidget->controller()->run_simulation();
    }

    void WaveformViewer::handleToggleMaxZoom()
    {
        if (!mCurrentWaveWidget) return;
        mCurrentWaveWidget->graphicCanvas()->toggleZoom();
    }

    void WaveformViewer::handleUndoZoomShift()
    {
        if (!mCurrentWaveWidget) return;
        mCurrentWaveWidget->graphicCanvas()->undoZoom();
    }

    void WaveformViewer::handleAddResultWave()
    {
        if (!mCurrentWaveWidget) return;
        mCurrentWaveWidget->addResults();
        mAddResultWaveAction->setEnabled(mCurrentWaveWidget->canImportWires());
    }

    void WaveformViewer::handleClockSet()
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

    void WaveformViewer::handleSelectGates()
    {
        if (!mCurrentWaveWidget) return;
        GateSelectionDialog gsd(this);
        if (gsd.exec() != QDialog::Accepted) return;

        mCurrentWaveWidget->setGates(gsd.selectedGates());
    }

    void WaveformViewer::setGates(std::vector<Gate*> gates)
    {
        if (!mCurrentWaveWidget) return;
        mCurrentWaveWidget->setGates(gates);
    }

    void WaveformViewer::handleSelectionChanged(void* sender)
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
