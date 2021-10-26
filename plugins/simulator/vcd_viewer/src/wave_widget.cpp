#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_label.h"
#include "netlist_simulator_controller/wave_data.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "vcd_viewer/wave_selection_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QDebug>

#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "hal_core/netlist/grouping.h"
#include "gui/content_manager/content_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/gui_globals.h"

namespace hal {

    WaveWidget::WaveWidget(NetlistSimulatorController *ctrl, QWidget *parent)
        : QSplitter(parent), mController(ctrl), mControllerOwner(nullptr),
          mWaveIndex(ctrl->get_waves()), mVisualizeNetState(false)
    {
        mFrame = new QFrame(this);
        mFrame->setLineWidth(3);
        mFrame->setFrameStyle(QFrame::Sunken);
        mFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        addWidget(mFrame);
        mWaveView = new WaveView(this);
        mWaveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mWaveScene = new WaveScene(&mWaveIndex, mWaveView);

        connect(mWaveView,&WaveView::changedXscale,mWaveScene,&WaveScene::xScaleChanged);
        mWaveView->setScene(mWaveScene);

        connect(mWaveScene,&WaveScene::cursorMoved,this,&WaveWidget::handleCursorMoved);
        connect(mWaveView,&WaveView::relativeYScroll,this,&WaveWidget::handleYScroll);
        connect(gContentManager->getSelectionDetailsWidget(),&SelectionDetailsWidget::triggerHighlight,this,&WaveWidget::handleSelectionHighlight);

        connect(&mWaveIndex,&WaveIndex::waveAppended,this,&WaveWidget::handleWaveAppended);
        connect(&mWaveIndex,&WaveIndex::waveRemoved,this,&WaveWidget::handleWaveRemoved);
        connect(&mWaveIndex,&WaveIndex::waveDataChanged,this,&WaveWidget::handleWaveDataChanged);

        connect(&mWaveIndex,&WaveIndex::waveAppended,mWaveScene,&WaveScene::handleWaveAppended);
        connect(&mWaveIndex,&WaveIndex::waveRemoved,mWaveScene,&WaveScene::handleWaveRemoved);
        connect(&mWaveIndex,&WaveIndex::waveDataChanged,mWaveScene,&WaveScene::handleWaveDataChanged);

        connect(mController, &NetlistSimulatorController::stateChanged, this, &WaveWidget::handleStateChanged);
    }

    WaveWidget::~WaveWidget()
    {
        disconnect(mWaveScene,&WaveScene::cursorMoved,this,&WaveWidget::handleCursorMoved);
        disconnect(mWaveView,&WaveView::relativeYScroll,this,&WaveWidget::handleYScroll);
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
        mWaveIndex.setAutoAddWaves(state < NetlistSimulatorController::SimulationRun);
        Q_EMIT stateChanged(state);
    }

    void WaveWidget::handleSelectionHighlight(const QVector<const SelectionTreeItem*>& highlight)
    {
        QSet<u32> hlIds;
        for (const SelectionTreeItem* sti : highlight)
            if (sti->itemType() == SelectionTreeItem::NetItem)
                hlIds.insert(sti->id());

        int inx = 0;
        for (auto it = mValues.begin(); it != mValues.end(); ++it)
        {
            (*it)->setHighlight(hlIds.contains(mWaveIndex.waveData(inx)->id()));
            ++inx;
        }
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
                    gtm->recolorGrouping(grp->get_id(),QColor(WaveLabel::sStateColor[i]));
                }
                mGroupIds[i] = grp->get_id();
            }
            visualizeCurrentNetState(mWaveScene->cursorPos());
        }
        else
        {
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
        QSet<int> alreadyShown = mWaveIndex.waveDataIndexSet();
        int n = mWaveIndex.waveDataList()->size();
        QMap<const WaveData*,int> wdMap;
        for (int i=0; i<n; i++)
        {
            if (alreadyShown.contains(i)) continue;
            wdMap.insert(mWaveIndex.waveDataList()->at(i),i);
        }
        WaveSelectionDialog wsd(wdMap,this);
        if (wsd.exec() != QDialog::Accepted) return;
        for (int i : wsd.selectedWaveIndices())
        {
            mWaveIndex.addWave(i);
        }
    }

    void WaveWidget::deleteWave(int dataIndex)
    {
        WaveLabel* wl = mValues.at(dataIndex);
        mValues.removeAt(dataIndex);
        mWaveIndex.removeIndex(dataIndex);
        wl->deleteLater();
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
        int inx = mValues.size();

        WaveLabel* wl = new WaveLabel(inx,wd->name(),mFrame);
        connect(wl,&WaveLabel::doubleClicked,this,&WaveWidget::editWaveData);
        connect(wl,&WaveLabel::triggerDelete,this,&WaveWidget::deleteWave);
        connect(wl,&WaveLabel::triggerSwap,this,&WaveWidget::handleLabelSwap);
        connect(wl,&WaveLabel::triggerMove,this,&WaveWidget::handleLabelMove);
        wl->setFixedWidth(250);
        wl->show();
        mValues.append(wl);
        updateLabel(inx,mWaveScene->cursorPos());
        resizeEvent(nullptr);
    }

    void WaveWidget::handleWaveDataChanged(int inx)
    {
        int i0 = inx < 0 ? 0 : inx;
        int i1 = inx < 0 ? mValues.size() : inx + 1;
        for (int i=i0; i<i1; i++)
            mValues.at(i)->setText(mWaveIndex.waveData(i)->name());
    }

    void WaveWidget::handleWaveRemoved(int inx)
    {
        if (inx < 0)
        {
            for (WaveLabel* wl : mValues)
                delete wl;
            mValues.clear();
        }
        else
        {
            delete mValues.at(inx);
            mValues.removeAt(inx);
        }
    }

    void WaveWidget::resizeEvent(QResizeEvent *event)
    {
        int w = event ? event->size().width() : width();
        int maxw = w / 2;
        int minw = 10;
        int n = mValues.size();
        for (int i=0; i<n; i++)
            if (mValues.at(i)->width()>minw)
                minw = mValues.at(i)->width();
        minw += 6;
        if (minw > maxw) minw = maxw;
        setSizes(QList<int>() << minw << w - minw );
    }

    void WaveWidget::editWaveData(int dataIndex)
    {
        const WaveData* editorInput = mWaveIndex.waveData(dataIndex);
        if (!editorInput) return;
        WaveData wd(*editorInput);
        WaveEditDialog wed(wd,this);
        if (wed.exec() != QDialog::Accepted) return;
        mWaveScene->setWaveData(dataIndex,wed.dataFactory()); // TODO : sanitize input data
    }

    void WaveWidget::visualizeCurrentNetState(float xpos)
    {
        QSet<Net*> netState[3]; // x, 0, 1
        for (int i=0; i<mWaveIndex.numberWavesShown(); i++)
        {
            const WaveData* wd = mWaveIndex.waveData(i);
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

    void WaveWidget::updateLabel(int dataIndex, float xpos)
    {
        WaveLabel* wl = mValues.at(dataIndex);
        if (wl->state()>=2) return;
        QPoint pos = mWaveView->mapFromScene(QPointF(xpos,mWaveScene->yPosition(dataIndex)-1));
        pos.setX(3);
        wl->setValue(mWaveIndex.waveData(dataIndex)->intValue(xpos));
        wl->move(pos);
    }

    void WaveWidget::handleYScroll(int dy)
    {
        for (WaveLabel* wl : mValues)
            wl->move(3,wl->pos().y() + dy);
    }

    void WaveWidget::handleCursorMoved(float xpos)
    {
        mWaveView->handleCursorMoved(xpos);

        for (int i=0; i<mWaveIndex.numberWavesShown(); i++)
            updateLabel(i,xpos);

        if (mVisualizeNetState) visualizeCurrentNetState(xpos);
    }

    void WaveWidget::handleLabelMove(int isource, int ypos)
    {
        int itarget = targetIndex(ypos);
        int n = mValues.size();
        for (int i=0; i<n; i++)
        {
            if (i == isource) continue;
            WaveLabel* wl = mValues.at(i);
            wl->setState(i == itarget ? 2 : 0);
            wl->update();
        }
        update();
    }

    int WaveWidget::targetIndex(int ypos)
    {
        int n = mValues.size() -1;
        if (n <= 0) return -1;
        int labh = mValues.at(0)->height();
        int y0 = + mValues.at(0)->pos().y();
        int yn = + mValues.at(n)->pos().y();

        int retval = (int) floor ( (1.5 + labh + ypos - y0) * n / (yn - y0));
        if (retval > n) return -1;
        return retval;
    }

    void WaveWidget::handleLabelSwap(int isource, int ypos)
    {

        int n = mValues.size();
        for (int i=0; i<n; i++)
            mValues.at(i)->setState(0);

        int itarget = targetIndex(ypos);
        if (itarget >= 0 && itarget != isource)
        {
            WaveLabel* wl = mValues.at(isource);
            mValues.removeAt(isource);
            mValues.insert(itarget < isource ? itarget : itarget-1, wl);
            mWaveIndex.move(isource,itarget);
        }
        for (int i=0; i<n; i++)
            mValues.at(i)->update();
        update();
    }

}
