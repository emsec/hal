#include "vcd_viewer/wave_tree_view.h"
#include "vcd_viewer/wave_tree_model.h"
#include "vcd_viewer/wave_widget.h"
#include "vcd_viewer/wave_edit_dialog.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QScrollBar>
#include <QDragMoveEvent>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QMimeData>

namespace hal {
    WaveTreeView::WaveTreeView(WaveDataList* wdList, QWidget* parent)
        : QTreeView(parent), mWaveDataList(wdList)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setUniformRowHeights(true);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        setDragDropMode(QAbstractItemView::DragDrop);
        setDragEnabled(true);
        setDropIndicatorShown(true);
        viewport()->setAcceptDrops(true);
        setMouseTracking(true);
        setAcceptDrops(true);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this,&QTreeView::expanded,this,&WaveTreeView::handleExpandCollapse);
        connect(this,&QTreeView::collapsed,this,&WaveTreeView::handleExpandCollapse);
        connect(this,&QTreeView::customContextMenuRequested,this,&WaveTreeView::handleContextMenuRequested);
    }

    void WaveTreeView::mouseDoubleClickEvent(QMouseEvent *event)
    {
        QModelIndex inx = indexAt(event->pos());
        if (inx.isValid())
        {
            WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
            WaveData* wd = nullptr;
            int iwave = wtm->waveIndex(inx);
            if (iwave >= 0)
            {
                wd = mWaveDataList->at(iwave);
            }
            else
            {
                int grpId = wtm->groupId(inx);
                if (grpId >= 0)
                    wd = mWaveDataList->mDataGroups.value(grpId);
            }
            if (wd)
            {
                WaveEditDialog wed(wd, this);
                if (wed.exec() == QDialog::Accepted)
                {
                    wd->setData(wed.dataMap());
                    mWaveDataList->updateWaveData(iwave);
                }
                return;
            }
        }
        QAbstractItemView::mouseDoubleClickEvent(event);
    }

    void WaveTreeView::scrollContentsBy(int dx, int dy)
    {
        QTreeView::scrollContentsBy(dx,dy);
        if (dy)
        {
            WaveWidget* ww = static_cast<WaveWidget*>(parent());
            ww->scrollToYpos(verticalScrollBar()->value());
        }
    }

    bool WaveTreeView::viewportEvent(QEvent* evt)
    {
        if (evt->type() == QEvent::Resize)
        {
            QAbstractItemView::viewportEvent(evt);
            Q_EMIT viewportHeightChanged(verticalScrollBar()->maximum());
            return true;
        }
        return QAbstractItemView::viewportEvent(evt);
    }

    void WaveTreeView::resizeEvent(QResizeEvent *event)
    {
        QAbstractItemView::resizeEvent(event);
        Q_EMIT sizeChanged(event->size().height(),viewport()->height());
    }

    void WaveTreeView::handleContextMenuRequested(const QPoint& pos)
    {
        mContextIndex = indexAt(pos);
        if (!mContextIndex.isValid()) return;
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        QMenu* menu = new QMenu(this);
        QAction* act;

        act = menu->addAction("Insert new group");
        connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertGroup);

        if (wtm->isLeaveItem(mContextIndex))
        {
            act = menu->addAction("Rename net " + wtm->netName(mContextIndex));
            act = menu->addAction("Edit net wave " + wtm->netName(mContextIndex));
            act = menu->addAction("Remove net wave " + wtm->netName(mContextIndex));
            connect(act,&QAction::triggered,this,&WaveTreeView::handleRemoveItem);
        }
        else
        {
            act = menu->addAction("View group data " + wtm->netName(mContextIndex));
            act = menu->addAction("Remove group " + wtm->netName(mContextIndex));
            connect(act,&QAction::triggered,this,&WaveTreeView::handleRemoveGroup);

            QMenu* menuBase = menu->addMenu("Value format for " + wtm->netName(mContextIndex));
            act = menuBase->addAction("Binary");
            act->setData(2);
            connect(act,&QAction::triggered,this,&WaveTreeView::handleSetValueFormat);
            act = menuBase->addAction("Decimal (signed)");
            act->setData(-10);
            connect(act,&QAction::triggered,this,&WaveTreeView::handleSetValueFormat);
            act = menuBase->addAction("Decimal (unsigned)");
            act->setData(10);
            connect(act,&QAction::triggered,this,&WaveTreeView::handleSetValueFormat);
            act = menuBase->addAction("Hexdec");
            act->setData(16);
            connect(act,&QAction::triggered,this,&WaveTreeView::handleSetValueFormat);
        }
        menu->popup(viewport()->mapToGlobal(pos));
    }

    void WaveTreeView::handleSetValueFormat()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        int grpId = wtm->groupId(mContextIndex);
        if (grpId<0) return;
        WaveDataGroup* grp = mWaveDataList->mDataGroups.value(grpId);
        if (!grp) return;

        QAction* act = static_cast<QAction*>(sender());
        grp->setValueBase(act->data().toInt());
        wtm->handleUpdateValueFormat();
    }

    void WaveTreeView::handleRemoveItem()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        wtm->removeItem(mContextIndex.row(),mContextIndex.parent());
        reorder();
    }

    void WaveTreeView::handleRemoveGroup()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        wtm->removeGroup(mContextIndex);
        expandAll();
        reorder();
    }

    void WaveTreeView::handleInsertGroup()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        wtm->insertGroup(mContextIndex);
        reorder();
    }

    void WaveTreeView::startDrag(Qt::DropActions supportedActions)
    {
        QModelIndexList sel = selectedIndexes();
        if (sel.isEmpty()) return;
        static_cast<WaveTreeModel*>(model())->setDragIndex(sel.at(0));
        QAbstractItemView::startDrag(supportedActions);
    }

    void WaveTreeView::handleExpandCollapse(const QModelIndex& index)
    {
        Q_UNUSED(index);

        reorder();
    }

    void WaveTreeView::handleInserted(const QModelIndex& index)
    {
        reorder();
        selectionModel()->setCurrentIndex(index,
                                          QItemSelectionModel::Clear  |
                                          QItemSelectionModel::Select |
                                          QItemSelectionModel::Rows );
    }


    void WaveTreeView::reorder()
    {
        mItemOrder.clear();

        orderRecursion(QModelIndex());
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        QHash<int,int> wavePositionMap;
        QHash<int,int> groupPositionMap;
        for (int i=0; i<mItemOrder.size(); i++)
        {
            int iwave = wtm->waveIndex(mItemOrder.at(i));
            if (iwave < 0)
            {
                int groupId = wtm->groupId(mItemOrder.at(i));
                if (groupId > 0)
                    groupPositionMap[groupId] = i;
            }
            else
                wavePositionMap[iwave] = i;
        }
        Q_EMIT reordered(wavePositionMap, groupPositionMap);
    }

    void WaveTreeView::orderRecursion(const QModelIndex &parent)
    {
        int n = model()->rowCount(parent);
        for (int i=0; i<n; i++)
        {
            QModelIndex inx = model()->index(i,0,parent);
            if (!inx.isValid()) continue;
            mItemOrder.append(inx);
            if (isExpanded(inx)) orderRecursion(inx);
        }
    }

    void WaveTreeView::setWaveSelection(const QSet<u32>& netIds)
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        selectionModel()->clear();
        for (QModelIndex inx : wtm->indexes(netIds))
        {
            selectionModel()->select(inx, QItemSelectionModel::Select);
        }
    }
}
