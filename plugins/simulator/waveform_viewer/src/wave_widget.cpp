#include "waveform_viewer/wave_widget.h"
#include "waveform_viewer/wave_tree_view.h"
#include "waveform_viewer/wave_tree_model.h"
#include "waveform_viewer/wave_graphics_canvas.h"
#include "waveform_viewer/waveform_viewer.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "waveform_viewer/wave_edit_dialog.h"
#include "waveform_viewer/wave_selection_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/utilities/log.h"
#include "gui/content_manager/content_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/gui_globals.h"

namespace hal {

    WaveWidget::WaveWidget(NetlistSimulatorController *ctrl, QWidget *parent)
        : QSplitter(parent), mController(ctrl), mControllerOwner(nullptr),
          mOngoingYscroll(false), mVisualizeNetState(false), mAutoAddWaves(true)
    {
        mWaveItemHash = new WaveItemHash;
        mWaveDataList = ctrl->get_waves();
        mTreeView     = new WaveTreeView(mWaveDataList,  mWaveItemHash, this);
        mTreeModel    = new WaveTreeModel(mWaveDataList, mWaveItemHash, this);
        mTreeView->setModel(mTreeModel);
        mTreeView->expandAll();
        mTreeView->setColumnWidth(0,200);
        mTreeView->setColumnWidth(1,40);
        mTreeView->setColumnWidth(2,60);
        mTreeView->header()->setStretchLastSection(true);
        addWidget(mTreeView);

        mScrollArea = new WaveGraphicsCanvas(mWaveDataList,  mWaveItemHash, this);
        addWidget(mScrollArea);

        connect(mWaveDataList,&WaveDataList::waveAdded,mTreeModel,&WaveTreeModel::handleWaveAdded);
        connect(mWaveDataList,&WaveDataList::groupAdded,mTreeModel,&WaveTreeModel::handleGroupAdded);
        connect(mWaveDataList,&WaveDataList::waveAddedToGroup,mTreeModel,&WaveTreeModel::handleWaveAddedToGroup);
        connect(mWaveDataList,&WaveDataList::groupAboutToBeRemoved,mTreeModel,&WaveTreeModel::handleGroupAboutToBeRemoved);
        connect(mWaveDataList,&WaveDataList::nameUpdated,mTreeModel,&WaveTreeModel::handleNameUpdated);
        connect(mWaveDataList,&WaveDataList::groupUpdated,mTreeModel,&WaveTreeModel::handleGroupUpdated);
//        connect(mWaveDataList,&WaveDataList::waveUpdated,mScene,&WaveScene::handleWaveUpdated);
        connect(mTreeModel,&WaveTreeModel::inserted,mTreeView,&WaveTreeView::handleInserted);
        connect(mTreeModel,&WaveTreeModel::triggerReorder,mTreeView,&WaveTreeView::reorder);
//        connect(mTreeView,&WaveTreeView::viewportHeightChanged,mGraphicsView,&WaveGraphicsView::handleViewportHeightChanged);
//        connect(mTreeView,&WaveTreeView::sizeChanged,mGraphicsView,&WaveGraphicsView::handleSizeChanged);
//        connect(mTreeView,&WaveTreeView::triggerUpdateWaveItems,mScene,&WaveScene::updateWaveItems);
//        connect(mTreeView,&WaveTreeView::numberVisibleChanged,mGraphicsView,&WaveGraphicsView::handleNumberVisibileChanged);
//        connect(mTreeView,&WaveTreeView::valueBaseChanged,mScene,&WaveScene::updateWaveItemValues);
//        connect(mGraphicsView,&WaveGraphicsView::changedXscale,mScene,&WaveScene::xScaleChanged);
//        connect(mScene,&WaveScene::cursorMoved,mTreeModel,&WaveTreeModel::handleCursorMoved);
//        connect(mWaveDataList,&WaveDataList::timeframeChanged,mScene,&WaveScene::handleTimeframeChanged);
//        connect(mWaveDataList,&WaveDataList::timeframeChanged,mGraphicsView,&WaveGraphicsView::handelTimeframeChanged);
        connect(mWaveDataList,&WaveDataList::triggerBeginResetModel,mTreeModel,&WaveTreeModel::forwardBeginResetModel);
        connect(mWaveDataList,&WaveDataList::triggerEndResetModel,mTreeModel,&WaveTreeModel::forwardEndResetModel);


        if (parent && parent->parent())   // VcdViewer -> QTabWidget -> WaveWidget
        {
            VcdViewer* vv = static_cast<VcdViewer*>(parent->parent());
            if (vv) connect(mController,&NetlistSimulatorController::parseComplete,vv,&VcdViewer::handleParseComplete);
        }

        connect(gContentManager->getSelectionDetailsWidget(),&SelectionDetailsWidget::triggerHighlight,this,&WaveWidget::handleSelectionHighlight);

        connect(mController, &NetlistSimulatorController::stateChanged, this, &WaveWidget::handleStateChanged);
        mTreeView->verticalScrollBar()->setValue(0);
        setSizes({320,880});
    }

    WaveWidget::~WaveWidget()
    {
//        disconnect(mScene,&WaveScene::cursorMoved,this,&WaveWidget::handleCursorMoved);
    }

    void WaveWidget::refreshNetNames()
    {
        int nWaves = mWaveDataList->size();
        for (int i=0; i<nWaves; i++)
        {
            const WaveData* wd = mWaveDataList->at(i);
            if (!wd->id()) continue;
            Net* n = gNetlist->get_net_by_id(wd->id());
            if (!n) continue;
            QString netName = QString::fromStdString(n->get_name());
            if (netName == wd->name()) continue;
            mWaveDataList->updateWaveName(i,netName);
        }
    }

    u32 WaveWidget::controllerId() const
    {
        if (!mController) return 0;
        return mController->get_id();
    }

    NetlistSimulatorController::SimulationState WaveWidget::state() const
    {
        if (!mController) return NetlistSimulatorController::NoGatesSelected;
        return mController->get_state();
    }

    void WaveWidget::takeOwnership(std::unique_ptr<NetlistSimulatorController>& ctrl)
    {
        mControllerOwner = std::move(ctrl);
    }

    bool WaveWidget::canImportWires() const
    {
        if (mWaveDataList->size() > mWaveItemHash->importedWires()) return true;
        if (mController)
        {
            SaleaeDirectory sd(mController->get_saleae_directory_filename());
            if (sd.get_next_available_index() > mWaveItemHash->importedWires()) return true;
        }
        return false;
    }

    bool WaveWidget::triggerClose()
    {
        if (!mControllerOwner) return false;
        deleteLater();
        return true;
    }

    void WaveWidget::handleStateChanged(NetlistSimulatorController::SimulationState state)
    {
        /*
        if (mAutoAddWaves && state >= NetlistSimulatorController::SimulationRun)
        {
            qDebug() << "disconnect WaveDataList::waveAdded" << hex << (quintptr) mWaveDataList << (quintptr) mTreeModel;
            disconnect(mWaveDataList,&WaveDataList::waveAdded,mTreeModel,&WaveTreeModel::handleWaveAdded);
            mAutoAddWaves = false;
        }
*/
        Q_EMIT stateChanged(state);
        qApp->processEvents();
    }

    void WaveWidget::handleSelectionHighlight(const QVector<const SelectionTreeItem*>& highlight)
    {
        QSet<u32> hlIds;
        for (const SelectionTreeItem* sti : highlight)
            if (sti->itemType() == SelectionTreeItem::NetItem)
                hlIds.insert(sti->id());

        mTreeView->setWaveSelection(hlIds);
    }

    void WaveWidget::createEngine(const QString &engineFactoryName)
    {
        mController->create_simulation_engine(engineFactoryName.toStdString());
    }

    void WaveWidget::setVisualizeNetState(bool state, bool activeTab)
    {
        /* TODO visualize net state
        GroupingTableModel* gtm = gContentManager->getGroupingManagerWidget()->getModel();
        static const char* grpNames[3] = {"x state", "0 state", "1 state"};
        mVisualizeNetState = state;
        if (!activeTab) return;
        if (mVisualizeNetState)
        {
            for (int i=0; i<3; i++)
            {
                Grouping* grp = gtm->groupingByName(grpNames[i]);
                if (!grp)
                {
                    grp = gNetlist->create_grouping(grpNames[i]);
                    gtm->recolorGrouping(grp->get_id(),QColor(WaveTreeModel::sStateColor[i]));
                }
                mGroupIds[i] = grp->get_id();
            }
            visualizeCurrentNetState(mScene->cursorXposition());
            connect(mScene,&WaveScene::cursorMoved,this,&WaveWidget::visualizeCurrentNetState);
        }
        else
        {
            disconnect(mScene,&WaveScene::cursorMoved,this,&WaveWidget::visualizeCurrentNetState);
            for (int i=0; i<3; i++)
            {
                Grouping* grp = gtm->groupingByName(grpNames[i]);
                if (grp) gNetlist->delete_grouping(grp);
                mGroupIds[i] = 0;
            }
        }
        */
    }

    void WaveWidget::addResults()
    {
        if (!canImportWires()) return;
        QSet<int> alreadyShownInx = mTreeModel->waveDataIndexSet();
        QSet<QString> alreadyShownNames;

        // Which wavedata container entries are not shown yet
        int n = mWaveDataList->size();
        QMap<WaveSelectionEntry,int> wseMap;
        for (int i=0; i<n; i++)
        {
            if (alreadyShownInx.contains(i))
                alreadyShownNames.insert(mWaveDataList->at(i)->name());
            else
            {
                WaveData* wd = mWaveDataList->at(i);
                wseMap.insert(WaveSelectionEntry(wd->id(),wd->name(),wd->data().size()),i);
            }
        }

        // Which salea directory entries are not shown yet
        SaleaeDirectory* sd = mController ? new SaleaeDirectory(mController->get_saleae_directory_filename()) : nullptr;
        if (sd)
        {
            for (const SaleaeDirectory::ListEntry& sdle : sd->get_net_list())
            {
                QString netName = QString::fromStdString(sdle.name);
                if (alreadyShownNames.contains(netName)) continue; // already shown
                if (mWaveDataList->waveIndexByNetId(sdle.id)>=0) continue; // already added to selection list by previous loop
                wseMap.insert(WaveSelectionEntry(sdle.id,netName,sdle.size),-1);
            }
        }

        WaveSelectionDialog wsd(wseMap,this);
        if (wsd.exec() == QDialog::Accepted)
        {
            wseMap = wsd.selectedWaves();

            for (auto it = wseMap.constBegin(); it!=wseMap.constEnd(); ++it)
            {
                int iwave = it.value();
                if (iwave<0 && sd)
                {
                    iwave = mWaveDataList->size();
                    WaveData* wd = new WaveData(it.key().id(),it.key().name());
                    if (wd->loadSaleae(*sd, mWaveDataList->timeFrame()))
                    {
                        mWaveDataList->add(wd,false);
                        mTreeModel->handleWaveAdded(iwave);
                    }
                }
                else if (iwave >= 0)
                    mTreeModel->handleWaveAdded(iwave);
            }
        }
        if (sd) delete sd;
    }

    void WaveWidget::setGates(const std::vector<Gate*>& gats)
    {
        if (!mController) return;
        mController->reset();
        mController->add_gates(gats);
    }

    void WaveWidget::handleWaveAppended(WaveData *wd)
    {
        Q_ASSERT(wd);
        //TODO
    }

    void WaveWidget::resizeEvent(QResizeEvent* event)
    {
        QSplitter::resizeEvent(event);
        int w = event->size().width() - 320;
        if (w < 100) return;
        setSizes({320,w});
    }

    void WaveWidget::handleEngineFinished(bool success)
    {
        if (!success) return;
        if (!mController->get_results())
            log_warning(mController->get_name(), "Cannot get simulation results");
    }

    void WaveWidget::visualizeCurrentNetState(float xpos)
    {
        QSet<Net*> netState[3]; // x, 0, 1
        QList<int> iwaveList = mTreeModel->waveDataIndexSet().toList();
        for (int iwave : iwaveList)
        {
            const WaveData* wd = mWaveDataList->at(iwave);
            Net* n = gNetlist->get_net_by_id(wd->id());
            if (!n) continue;
            int tval = wd->intValue(xpos);
            if (tval < -1 || tval > 1) continue;
            netState[tval+1].insert(n);
        }

        for (int i=0; i<3; i++)
        {
            Grouping* grp = gNetlist->get_grouping_by_id(mGroupIds[i]);
            if (!grp) continue;
            for (Net* n : grp->get_nets())
            {
                auto it = netState[i].find(n);
                if (it == netState[i].end())
                    grp->remove_net(n);
                else
                    netState[i].erase(it);
            }
            for (Net* n : netState[i])
                grp->assign_net(n, true);
        }
    }

    void WaveWidget::scrollToYpos(int ypos)
    {
        if (mOngoingYscroll) return;
        mOngoingYscroll = true;
        if (mTreeView->verticalScrollBar()->value() != ypos)
            mTreeView->verticalScrollBar()->setValue(ypos);


        // TODO scroll to ypos
        /*
        if (mGraphicsView->verticalScrollBar()->value() != ypos)
        {
            mGraphicsView->verticalScrollBar()->setMaximum(
                        mTreeView->verticalScrollBar()->maximum());
            mGraphicsView->verticalScrollBar()->setValue(ypos);
        }
        */
        mOngoingYscroll = false;
    }

    void WaveWidget::removeGroup(u32 grpId)
    {
        if (!grpId) return;
        mWaveDataList->removeGroup(grpId);
    }
}
