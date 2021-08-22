#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_view.h"
#include "vcd_viewer/wave_scene.h"
#include "vcd_viewer/wave_label.h"
#include "vcd_viewer/wave_data.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "math.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QDebug>

#include "hal_core/netlist/grouping.h"
#include "gui/content_manager/content_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/gui_globals.h"

namespace hal {

    WaveWidget::WaveWidget(QWidget *parent)
        : QSplitter(parent), mVisualizeNetState(false)
    {
        mFrame = new QFrame(this);
        mFrame->setLineWidth(3);
        mFrame->setFrameStyle(QFrame::Sunken);
        mFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        addWidget(mFrame);
        mWaveView = new WaveView(this);
        mWaveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mWaveScene = new WaveScene(mWaveView);

        connect(mWaveView,&WaveView::changedXscale,mWaveScene,&WaveScene::xScaleChanged);
        mWaveView->setScene(mWaveScene);

        connect(mWaveScene,&WaveScene::cursorMoved,this,&WaveWidget::handleCursorMoved);
        connect(mWaveView,&WaveView::relativeYScroll,this,&WaveWidget::handleYScroll);
        connect(gContentManager->getSelectionDetailsWidget(),&SelectionDetailsWidget::triggerHighlight,this,&WaveWidget::handleSelectionHighlight);
    }

    void WaveWidget::updateIndices()
    {
        int n = mValues.size();
        float xpos = mWaveScene->cursorPos();
        for (int i=0; i<n; i++)
        {
            mValues.at(i)->setDataIndex(i);
            updateLabel(i,xpos);
        }
        mWaveIndices.clear();
        for (int i=0; i<n; i++)
            mWaveIndices.insert(mWaveScene->waveData(i)->id(),i);
    }

    void WaveWidget::handleSelectionHighlight(const QVector<const SelectionTreeItem*>& highlight)
    {
        QSet<u32> hlIds;
        for (const SelectionTreeItem* sti : highlight)
            if (sti->itemType() == SelectionTreeItem::NetItem)
                hlIds.insert(sti->id());
        for (auto it = mWaveIndices.constBegin(); it != mWaveIndices.constEnd(); ++it)
        {
            mValues.at(it.value())->setHighlight(hlIds.contains(it.key()));
        }
    }

    void WaveWidget::setVisualizeNetState(bool state)
    {
        GroupingTableModel* gtm = gContentManager->getGroupingManagerWidget()->getModel();
        static const char* grpNames[3] = {"x state", "0 state", "1 state"};
        static const char* grpColor[3] = {"#707071", "#102080", "#802010"};
        if (state == mVisualizeNetState) return;
        if ((mVisualizeNetState = state))
        {
            for (int i=0; i<3; i++)
            {
                Grouping* grp = gtm->groupingByName(grpNames[i]);
                if (!grp)
                {
                    grp = gNetlist->create_grouping(grpNames[i]);
                    gtm->recolorGrouping(grp->get_id(),QColor(grpColor[i]));
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

    void WaveWidget::deleteWave(int dataIndex)
    {
        WaveLabel* wl = mValues.at(dataIndex);
        mValues.removeAt(dataIndex);
        mWaveScene->deleteWave(dataIndex);
        updateIndices();
        wl->deleteLater();
    }

    void WaveWidget::addOrReplaceWave(WaveData* wd)
    {
        auto it = mWaveIndices.find(wd->id());
        if (it != mWaveIndices.end())
        {
            mWaveScene->setWaveData(it.value(),wd);
            return;
        }
        int inx = mWaveScene->addWave(wd);
        Q_ASSERT(mValues.size() == inx);
        WaveLabel* wl = new WaveLabel(inx,mWaveScene->waveData(inx)->name(),mFrame);
        connect(wl,&WaveLabel::doubleClicked,this,&WaveWidget::editWaveData);
        connect(wl,&WaveLabel::triggerDelete,this,&WaveWidget::deleteWave);
        connect(wl,&WaveLabel::triggerSwap,this,&WaveWidget::handleLabelSwap);
        connect(wl,&WaveLabel::triggerMove,this,&WaveWidget::handleLabelMove);
        wl->setFixedWidth(250);
        wl->show();
        mValues.append(wl);
        mWaveIndices.insert(wd->id(),inx);
        updateLabel(inx,mWaveScene->cursorPos());
        resizeEvent(nullptr);
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

    const WaveData* WaveWidget::waveDataByNetId(u32 id) const
    {
        int inx = mWaveIndices.value(id,-1);
        if (inx < 0) return nullptr;
        return mWaveScene->waveData(inx);
    }

    void WaveWidget::editWaveData(int dataIndex)
    {
        const WaveData* editorInput = mWaveScene->waveData(dataIndex);
        if (!editorInput) return;
        WaveData wd(*editorInput);
        WaveEditDialog wed(wd,this);
        if (wed.exec() != QDialog::Accepted) return;
        mWaveScene->setWaveData(dataIndex,wed.dataFactory()); // TODO : sanitize input data
    }

    void WaveWidget::visualizeCurrentNetState(float xpos)
    {
        QSet<Net*> netState[3]; // x, 0, 1
        for (int i=0; i<mWaveScene->numberWaves(); i++)
        {
            const WaveData* wd = mWaveScene->waveData(i);
            Net* n = gNetlist->get_net_by_id(wd->id());
            if (!n) continue;
            int tval = wd->tValue(xpos);
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
        wl->setValue(mWaveScene->waveData(dataIndex)->tValue(xpos));
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

        for (int i=0; i<mWaveScene->numberWaves(); i++)
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
            mWaveScene->moveToIndex(isource,itarget);
            updateIndices();
        }
        for (int i=0; i<n; i++)
            mValues.at(i)->update();
        update();
    }

}
