#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_tree_view.h"
#include "vcd_viewer/wave_tree_model.h"
#include "vcd_viewer/wave_graphics_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_item.h"
#include "netlist_simulator_controller/wave_data.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "vcd_viewer/wave_selection_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QHeaderView>
#include <QDebug>

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
        mWaveDataList = ctrl->get_waves();
        mTreeView     = new WaveTreeView(mWaveDataList,  this);
        mTreeModel    = new WaveTreeModel(mWaveDataList, this);
        mTreeView->setModel(mTreeModel);
        mTreeView->expandAll();
        mTreeView->setColumnWidth(0,200);
        mTreeView->setColumnWidth(1,40);
        mTreeView->setColumnWidth(2,60);
        mTreeView->header()->setStretchLastSection(true);
        addWidget(mTreeView);

        mGraphicsView = new WaveGraphicsView(this);
        mScene = new WaveScene(mWaveDataList, this);
        mGraphicsView->setScene(mScene);
        mGraphicsView->setDefaultTransform();
        addWidget(mGraphicsView);

        connect(mWaveDataList,&WaveDataList::waveAdded,mTreeModel,&WaveTreeModel::handleWaveAdded);
        connect(mWaveDataList,&WaveDataList::groupAdded,mTreeModel,&WaveTreeModel::handleGroupAdded);
        connect(mWaveDataList,&WaveDataList::waveMovedToGroup,mTreeModel,&WaveTreeModel::handleWaveMovedToGroup);
        connect(mWaveDataList,&WaveDataList::groupAboutToBeRemoved,mTreeModel,&WaveTreeModel::handleGroupAboutToBeRemoved);
        connect(mWaveDataList,&WaveDataList::nameUpdated,mTreeModel,&WaveTreeModel::handleNameUpdated);
        connect(mWaveDataList,&WaveDataList::waveUpdated,mScene,&WaveScene::handleWaveUpdated);
        connect(mTreeModel,&WaveTreeModel::inserted,mTreeView,&WaveTreeView::handleInserted);
        connect(mTreeModel,&WaveTreeModel::triggerReorder,mTreeView,&WaveTreeView::reorder);
//        connect(mWaveDataList,&WaveDataList::waveAdded,mScene,&WaveScene::handleWaveAdded);
        connect(mWaveDataList,&WaveDataList::groupAdded,mScene,&WaveScene::handleGroupAdded);
        connect(mTreeView,&WaveTreeView::viewportHeightChanged,mGraphicsView,&WaveGraphicsView::handleViewportHeightChanged);
        connect(mTreeView,&WaveTreeView::sizeChanged,mGraphicsView,&WaveGraphicsView::handleSizeChanged);
        connect(mTreeModel,&WaveTreeModel::indexRemoved,mScene,&WaveScene::handleIndexRemoved);
//        connect(mTreeModel,&WaveTreeModel::indexInserted,mScene,&WaveScene::handleIndexInserted);
        connect(mTreeView,&WaveTreeView::reordered,mScene,&WaveScene::setWavePositions);
        connect(mTreeView,&WaveTreeView::valueBaseChanged,mScene,&WaveScene::updateWaveItemValues);
        connect(mGraphicsView,&WaveGraphicsView::changedXscale,mScene,&WaveScene::xScaleChanged);
        connect(mScene,&WaveScene::cursorMoved,mTreeModel,&WaveTreeModel::handleCursorMoved);


        connect(gContentManager->getSelectionDetailsWidget(),&SelectionDetailsWidget::triggerHighlight,this,&WaveWidget::handleSelectionHighlight);

        /*
        connect(&mWaveIndex,&WaveIndex::waveAppended,this,&WaveWidget::handleWaveAppended);

        connect(&mWaveIndex,&WaveIndex::waveAppended,mWaveScene,&WaveScene::handleWaveAppended);
        connect(&mWaveIndex,&WaveIndex::waveRemoved,mWaveScene,&WaveScene::handleWaveRemoved);
        connect(&mWaveIndex,&WaveIndex::waveDataChanged,mWaveScene,&WaveScene::handleWaveDataChanged);
        connect(mWaveIndex.waveDataList(),&WaveDataList::maxTimeChanged,mWaveScene,&WaveScene::handleMaxTimeChanged);

        connect(&mWaveIndex,&WaveIndex::waveRemoved,mWaveView,&WaveView::handleWaveRemoved);
        connect(mWaveIndex.waveDataList(),&WaveDataList::maxTimeChanged,mWaveView,&WaveView::handleMaxTimeChanged);
*/
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
        int n = mWaveDataList->size();
        for (int i=0; i<n; i++)
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

    bool WaveWidget::triggerClose()
    {
        if (!mControllerOwner) return false;
        deleteLater();
        return true;
    }

    void WaveWidget::handleStateChanged(NetlistSimulatorController::SimulationState state)
    {
        if (mAutoAddWaves && state >= NetlistSimulatorController::SimulationRun)
        {
            qDebug() << "disconnect WaveDataList::waveAdded" << hex << (quintptr) mWaveDataList << (quintptr) mTreeModel;
            disconnect(mWaveDataList,&WaveDataList::waveAdded,mTreeModel,&WaveTreeModel::handleWaveAdded);
 //           disconnect(mWaveDataList,&WaveDataList::waveAdded,mScene,&WaveScene::handleWaveAdded);
            mAutoAddWaves = false;
        }

        Q_EMIT stateChanged(state);
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
    }

    void WaveWidget::addResults()
    {
        if (mController->get_state() != NetlistSimulatorController::ShowResults) return;
        QSet<int> alreadyShown = mTreeModel->waveDataIndexSet();
        int n = mWaveDataList->size();
        QMap<const WaveData*,int> wdMap;
        for (int i=0; i<n; i++)
        {
            if (alreadyShown.contains(i)) continue;
            wdMap.insert(mWaveDataList->at(i),i);
        }
        WaveSelectionDialog wsd(wdMap,this);
        if (wsd.exec() != QDialog::Accepted) return;
        for (int i : wsd.selectedWaveIndices())
        {
            mTreeModel->handleWaveAdded(i);
        }
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
        int h = event->size().height();
        if (w < 100) return;
        setSizes({320,w});
        mGraphicsView->setCursorPos(QPoint(w/3,3*h/4));
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
        if (mGraphicsView->verticalScrollBar()->value() != ypos)
            mGraphicsView->verticalScrollBar()->setValue(ypos);
        mOngoingYscroll = false;
    }

    u32 WaveWidget::addGroup(const std::string& name, const std::vector<u32>& netIds)
    {
 //       QVector<u32> nets = QVector<u32>(netIds.begin(),netIds.end());
        QVector<u32> nets;
        nets.reserve(netIds.size());
        for (u32 id : netIds) nets.append(id);
        return mWaveDataList->createGroup(QString::fromStdString(name),nets);
    }

    void WaveWidget::removeGroup(u32 grpId)
    {
        if (!grpId) return;
        mWaveDataList->removeGroup(grpId);
    }
}
