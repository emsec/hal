#include "waveform_viewer/wave_tree_view.h"
#include "waveform_viewer/wave_tree_model.h"
#include "waveform_viewer/wave_widget.h"
#include "waveform_viewer/wave_edit_dialog.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QScrollBar>
#include <QDragMoveEvent>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QInputDialog>
#include <QMimeData>
#include <QPainter>

namespace hal {
    WaveTreeView::WaveTreeView(WaveDataList* wdList, WaveItemHash *wHash, QWidget* parent)
        : QTreeView(parent), mWaveDataList(wdList), mWaveItemHash(wHash), mKillMode(false)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
        setItemDelegateForColumn(2,new WaveValueDelegate);
        connect(this,&QTreeView::expanded,this,&WaveTreeView::handleExpandCollapse);
        connect(this,&QTreeView::collapsed,this,&WaveTreeView::handleExpandCollapse);
        connect(this,&QTreeView::customContextMenuRequested,this,&WaveTreeView::handleContextMenuRequested);
    }

    void WaveTreeView::mouseDoubleClickEvent(QMouseEvent *event)
    {
        QModelIndex inx = indexAt(event->pos());
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        if (mKillMode)
        {
            QModelIndexList toDeleteInx;
            if (selectionModel()->hasSelection())
                toDeleteInx = selectionModel()->selectedRows(0);
            else
                toDeleteInx << inx;
            auto it = toDeleteInx.begin();
            while (it != toDeleteInx.end())
            {
                if (wtm->isLeaveItem(*it))
                {
                    wtm->removeItem(it->row(),it->parent());
                    it = toDeleteInx.erase(it);
                }
                else
                    ++it;
            }
            for (QModelIndex dinx : toDeleteInx)
            {
                WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(wtm->item(dinx));
                if (grp) wtm->removeGroup(dinx);
            }
        }
        else if (inx.isValid())
        {
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
        if (verticalScrollBar()->isVisible())
            Q_EMIT sizeChanged(viewport()->height(), verticalScrollBar()->maximum(), verticalScrollBar()->value());
        else
            Q_EMIT sizeChanged(viewport()->height(), -1, -1);
    }

    void WaveTreeView::toggleKillMode()
    {
        mKillMode = ! mKillMode;
        if (mKillMode)
        {
            setCursor(QCursor(QPixmap(":/icons/kill_cursor","PNG")));
            setSelectionMode(QAbstractItemView::MultiSelection);
        }
        else
        {
            unsetCursor();
            setSelectionMode(QAbstractItemView::SingleSelection);
            clearSelection();
        }
    }

    void WaveTreeView::handleContextMenuRequested(const QPoint& pos)
    {
        if (mKillMode)
        {
            toggleKillMode();
            return;
        }
        mContextIndex = indexAt(pos);
        if (!mContextIndex.isValid()) return;
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        QMenu* menu = new QMenu(this);
        QAction* act;

        act = menu->addAction("Insert new group");
        connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertGroup);

        act = menu->addAction("Rename '" + wtm->netName(mContextIndex) + "'");
        connect(act,&QAction::triggered,this,&WaveTreeView::handleRenameItem);

        if (wtm->isLeaveItem(mContextIndex))
        {
            act = menu->addAction("Edit net wave " + wtm->netName(mContextIndex));
            connect(act,&QAction::triggered,this,&WaveTreeView::handleEditOrBrowseItem);

            act = menu->addAction("Remove net wave " + wtm->netName(mContextIndex));
            connect(act,&QAction::triggered,this,&WaveTreeView::handleRemoveItem);
        }
        else
        {
            act = menu->addAction("View group data " + wtm->netName(mContextIndex));
            connect(act,&QAction::triggered,this,&WaveTreeView::handleEditOrBrowseItem);

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
        menu->addSeparator();
        act = menu->addAction("Remove multiple items by select & double click");
        connect(act,&QAction::triggered,this,&WaveTreeView::toggleKillMode);
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
        wtm->handleUpdateValueColumn();
        Q_EMIT valueBaseChanged();
    }

    void WaveTreeView::handleRenameItem()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveData* wd = wtm->item(mContextIndex);
        if (!wd) return;
        bool confirm;
        QString newName =
                QInputDialog::getText(this, "Change name in waveform list", "New name:", QLineEdit::Normal,
                                      wd->name(), &confirm);
        if (confirm && !newName.isEmpty())
            wd->setName(newName);
    }

    void WaveTreeView::handleEditOrBrowseItem()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveData* wd = wtm->item(mContextIndex);
        if (!wd) return;

        WaveEditDialog wed(wd, this);
        if (wed.exec() == QDialog::Accepted)
        {
            wd->setData(wed.dataMap());
            mWaveDataList->updateWaveData(mWaveDataList->waveIndexByNetId(wd->id()));
        }
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
        selectionModel()->setCurrentIndex(index,
                                          QItemSelectionModel::Clear  |
                                          QItemSelectionModel::Select |
                                          QItemSelectionModel::Rows );
        reorder();
    }


    void WaveTreeView::reorder()
    {
        mItemOrder.clear();
        orderRecursion(QModelIndex());

        QSet<QModelIndex>sel = selectionModel()->selectedRows().toSet();
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveItemHash notPlaced = *mWaveItemHash;
        int nVisible = mItemOrder.size();
        if (nVisible != mWaveItemHash->visibleEntries())
        {
            mWaveItemHash->setVisibleEntries(nVisible);
            if (verticalScrollBar()->isVisible())
                Q_EMIT numberVisibleChanged(nVisible, verticalScrollBar()->maximum(), verticalScrollBar()->value());
            else
                Q_EMIT numberVisibleChanged(nVisible, -1, -1);
        }
        for (int i=0; i<nVisible; i++)
        {
            QModelIndex currentIndex = mItemOrder.at(i);
            int iwave = wtm->waveIndex(currentIndex);
            if (iwave < 0)
            {
                int groupId = wtm->groupId(currentIndex);
                if (groupId > 0)
                {
                    WaveItemIndex wii(groupId, WaveItemIndex::Group);
                    WaveItem* wi = mWaveItemHash->value(wii);
                    if (!wi)
                    {
                        mWaveItemHash->dump("Crash");
                        qDebug() << "group wii not found" << groupId;
                    }
                    Q_ASSERT(wi);
                    wi->setYposition(i);
                    wi->setWaveVisible(true);
                    wi->setWaveSelected(sel.contains(currentIndex));
                    auto it = notPlaced.find(wii);
                    if (it != notPlaced.end()) notPlaced.erase(it);
                }
            }
            else
            {
                int groupId = wtm->groupId(currentIndex.parent());
                if (groupId < 0) groupId = 0;
                WaveItemIndex wii(iwave, WaveItemIndex::Wire, groupId);
                WaveItem* wi = mWaveItemHash->value(wii);
                if (!wi)
                {
                    mWaveItemHash->dump("Crash");
                    qDebug() << "wire wii not found" << iwave << groupId;
                }
                Q_ASSERT(wi);
                wi->setYposition(i);
                wi->setWaveVisible(true);
                wi->setWaveSelected(sel.contains(currentIndex));
                auto it = notPlaced.find(wii);
                if (it != notPlaced.end()) notPlaced.erase(it);
            }
        }

        for (WaveItem* wi : notPlaced.values())
            wi->setWaveVisible(false);

        Q_EMIT triggerUpdateWaveItems();
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

    void WaveTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        if (previous.isValid())
        {
            WaveItem* wi = mWaveItemHash->value(wtm->hashIndex(previous));
            if (wi)
            {
                wi->setWaveSelected(false);
                Q_EMIT triggerUpdateWaveItems();
            }
        }
        if (current.isValid())
        {
            WaveItem* wi = mWaveItemHash->value(wtm->hashIndex(current));
            if (wi)
            {
                if (wi) wi->setWaveSelected(true);
                Q_EMIT triggerUpdateWaveItems();
            }
        }
    }

    void WaveTreeView::setWaveSelection(const QSet<u32>& netIds)
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        selectionModel()->clear();
        bool first = true;
        for (QModelIndex inx : wtm->indexes(netIds))
        {
            selectionModel()->select(inx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            if (first)
            {
                first = false;
                setCurrentIndex(inx);
                scrollTo(inx, EnsureVisible);
            }
        }
    }

    void WaveValueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                  const QModelIndex &index) const
    {
        QPen textPen = QPen(option.palette.text(),0);
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
            textPen = QPen(option.palette.highlightedText(),0);
        }
        QString txt = index.data(Qt::DisplayRole).toString();
        QRect rectTxt = option.rect;
        int w = rectTxt.width()-22;
        if (w > 30)
        {
            rectTxt.setWidth(w);
            QColor  col = index.data(Qt::BackgroundRole).value<QColor>();
            QRect rectCol(rectTxt.left()+w+3,rectTxt.top()+3,16,16);
            painter->setBrush(QBrush(col));
            painter->drawEllipse(rectCol);
        }
        painter->setPen(textPen);
        QFont font = painter->font();
        font.setPixelSize(12);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(rectTxt,Qt::AlignRight|Qt::AlignTop,txt);
    }
}
