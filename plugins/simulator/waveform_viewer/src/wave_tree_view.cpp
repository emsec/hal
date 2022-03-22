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
        : QTreeView(parent), mWaveDataList(wdList), mWaveItemHash(wHash)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setUniformRowHeights(true);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
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
        if (inx.isValid())
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

    void WaveTreeView::handleContextMenuRequested(const QPoint& pos)
    {
        mContextIndexList = sortedSelection();
        if (mContextIndexList.isEmpty())
        {
            QModelIndex inx = indexAt(pos);
            if (!inx.isValid()) return;
            mContextIndexList.append(inx);
        }
        bool singleSelection = mContextIndexList.size() == 1;

        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        QMenu* menu = new QMenu(this);
        QAction* act;

        if (singleSelection)
        {
            QString selName =  wtm->netName(mContextIndexList.at(0));
            act = menu->addAction("Insert new group");
            connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertGroup);

            act = menu->addAction("Rename '" + selName + "'");
            connect(act,&QAction::triggered,this,&WaveTreeView::handleRenameItem);

            if (wtm->isLeaveItem(mContextIndexList.at(0)))
            {
                act = menu->addAction("Edit net wave " + selName);
                connect(act,&QAction::triggered,this,&WaveTreeView::handleEditOrBrowseItem);

                act = menu->addAction("Remove net wave " + selName);
                connect(act,&QAction::triggered,this,&WaveTreeView::handleRemoveItem);
            }
            else
            {
                act = menu->addAction("View group data " + selName);
                connect(act,&QAction::triggered,this,&WaveTreeView::handleEditOrBrowseItem);

                act = menu->addAction("Remove group " + selName);
                connect(act,&QAction::triggered,this,&WaveTreeView::handleRemoveGroup);

                QMenu* menuBase = menu->addMenu("Value format for " + selName);
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
        }
        else
        {
            act = menu->addAction(QString("Remove %1 selected items").arg(mContextIndexList.size()));
            connect(act,&QAction::triggered,this,&WaveTreeView::handleRemoveMulti);
        }
        if (!mWaveDataList->mDataGroups.isEmpty())
        {
            menu->addSeparator();
            act = menu->addAction("Collapse all groups");
            connect(act,&QAction::triggered,this,&QTreeView::collapseAll);
            act = menu->addAction("Expand all groups");
            connect(act,&QAction::triggered,this,&QTreeView::expandAll);
        }
        menu->popup(viewport()->mapToGlobal(pos));
    }

    void WaveTreeView::handleSetValueFormat()
    {
        if (mContextIndexList.size() != 1) return;
        QModelIndex inx = mContextIndexList.at(0);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        int grpId = wtm->groupId(inx);
        if (grpId<0) return;
        WaveDataGroup* grp = mWaveDataList->mDataGroups.value(grpId);
        if (!grp) return;

        QAction* act = static_cast<QAction*>(sender());
        grp->setValueBase(act->data().toInt());
        wtm->handleUpdateValueColumn();

        if (grpId)
        {
            WaveItem* wi = mWaveItemHash->value(WaveItemIndex(grpId,WaveItemIndex::Group,0));
            if (wi) wi->setRequest(WaveItem::DataChanged);
        }
    }

    void WaveTreeView::handleRenameItem()
    {
        if (mContextIndexList.size() != 1) return;
        QModelIndex inx = mContextIndexList.at(0);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveData* wd = wtm->item(inx);
        if (!wd) return;
        bool confirm;
        QString newName =
                QInputDialog::getText(this, "Change name in waveform list", "New name:", QLineEdit::Normal,
                                      wd->name(), &confirm);
        if (confirm && !newName.isEmpty())
        {
            if (wd->rename(newName))
            {
                SaleaeDirectoryStoreRequest save(&mWaveDataList->saleaeDirectory());
                WaveDataGroup* wdg = dynamic_cast<WaveDataGroup*>(wd);
                if (wdg)
                {
                    SaleaeDirectoryGroupEntry* sdge = mWaveDataList->saleaeDirectory().get_group(wdg->id());
                    if (sdge) sdge->rename(newName.toStdString());
                }
                else
                    mWaveDataList->saleaeDirectory().rename_net(wd->id(),newName.toStdString());
            }
        }
    }

    void WaveTreeView::handleEditOrBrowseItem()
    {
        if (mContextIndexList.size()!=1) return;
        QModelIndex inx = mContextIndexList.at(0);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveData* wd = wtm->item(inx);
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
        if (mContextIndexList.size()!=1) return;
        QModelIndex inx = mContextIndexList.at(0);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveTreeModel::ReorderRequest req(wtm);
        mWaveItemHash->dispose(wtm->removeItemFromHash(inx.row(),inx.parent()));
    }

    void WaveTreeView::handleRemoveGroup()
    {
        if (mContextIndexList.size()!=1) return;
        QModelIndex inx = mContextIndexList.at(0);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveTreeModel::ReorderRequest req(wtm);
        wtm->removeGroup(inx);
//        expandAll();
    }

    void WaveTreeView::handleRemoveMulti()
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveTreeModel::ReorderRequest req(wtm);
        for (const QModelIndex& inx : mContextIndexList)
        {
            if (wtm->isLeaveItem(inx))
            {
                mWaveItemHash->dispose(wtm->removeItemFromHash(inx.row(),inx.parent()));
            }
            else
            {
                wtm->removeGroup(inx);
            }
        }
//        expandAll();
    }

    void WaveTreeView::handleInsertGroup()
    {
        if (mContextIndexList.size() != 1) return;
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        wtm->insertGroup(mContextIndexList.at(0));
    }

    QModelIndexList WaveTreeView::sortedSelection() const
    {
        QMap<int,QModelIndex> sortInx;
        for (const QModelIndex& inx : selectedIndexes())
        {
            if (inx.column()) continue;
            int sortCode = 2000000000;
            QModelIndex p = inx.parent();
            // subtract group unless root
            if (p.isValid() && p.internalPointer())
                sortCode -= (p.row()+1) * 200000;
            sortCode -= (inx.row()+1);
            sortInx.insert(sortCode,inx);
        }
        return sortInx.values();
    }

    void WaveTreeView::startDrag(Qt::DropActions supportedActions)
    {
        QModelIndexList sel = sortedSelection();
        if (sel.isEmpty()) return;
        static_cast<WaveTreeModel*>(model())->setDragIndexes(sel);
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
                auto it = notPlaced.find(wii);
                if (it != notPlaced.end()) notPlaced.erase(it);
            }
        }

        for (WaveItem* wi : notPlaced.values())
            wi->setWaveVisible(false);

        Q_EMIT triggerUpdateWaveItems();
    }

    void WaveTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
        QTreeView::selectionChanged(selected,deselected);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        for (WaveItem* wi : mWaveItemHash->values())
            wi->setWaveSelected(false);
        for (const QModelIndex& inx : sortedSelection())
        {
            WaveItemIndex wii = wtm->hashIndex(inx);
            WaveItem* wi = mWaveItemHash->value(wii);
            if (wi) wi->setWaveSelected(true);
        }
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
