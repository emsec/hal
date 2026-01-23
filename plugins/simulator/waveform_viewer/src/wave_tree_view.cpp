#include "waveform_viewer/wave_tree_view.h"
#include "waveform_viewer/wave_tree_model.h"
#include "waveform_viewer/wave_widget.h"
#include "waveform_viewer/wave_edit_dialog.h"
#include "waveform_viewer/boolean_dialog.h"
#include "waveform_viewer/trigger_dialog.h"
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
        connect(this,&QTreeView::expanded,this,&WaveTreeView::handleExpand);
        connect(this,&QTreeView::collapsed,this,&WaveTreeView::handleCollapse);
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
                editWaveData(wd);
                return;
            }
        }
        QAbstractItemView::mouseDoubleClickEvent(event);
    }

    void WaveTreeView::editWaveData(WaveData *wd)
    {
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveEditDialog wed(wd, wtm->cursorTime(), this);
        if (wed.exec() == QDialog::Accepted)
        {
            wd->setData(wed.dataMap());
            mWaveDataList->updateWaveData(mWaveDataList->waveIndexByNetId(wd->id()));
        }
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
        bool onlyRootSel = wtm->onlyRootItemsSelected(mContextIndexList);
        QMenu* menu = new QMenu(this);
        QAction* act;

        if (singleSelection)
        {
            QString selName =  wtm->netName(mContextIndexList.at(0));
            selName.replace(QChar('&'),"&&");
            act = menu->addAction("Insert new group");
            connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertGroup);

            act = menu->addAction("Insert new boolean expression");
            connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertBoolean);

            act = menu->addAction("Insert new trigger based on selection");
            connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertTrigger);

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
            if (onlyRootSel)
            {
                act = menu->addAction(QString("Boolean expression based on %1 selected items").arg(mContextIndexList.size()));
                connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertBoolean);
                act = menu->addAction(QString("Trigger time set based on %1 selected items").arg(mContextIndexList.size()));
                connect(act,&QAction::triggered,this,&WaveTreeView::handleInsertTrigger);
            }
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
                SaleaeDirectoryNetEntry::Type tp = wd->composedType();

                if (tp != SaleaeDirectoryNetEntry::None)
                {
                    SaleaeDirectoryComposedEntry sdce = mWaveDataList->saleaeDirectory().get_composed(wd->id(),tp);
                    if (!sdce.isNull())
                    {
                        sdce.rename(newName.toStdString());
                        mWaveDataList->saleaeDirectory().add_or_replace_composed(sdce);
                    }
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
        editWaveData(wd);
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
        if (mContextIndexList.isEmpty()) return;
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveTreeModel::ReorderRequest req(wtm);
        SaleaeDirectoryStoreRequest save(&mWaveDataList->saleaeDirectory());
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

    void WaveTreeView::handleInsertBoolean()
    {
        if (mContextIndexList.empty()) return;
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        QList<WaveData*> selectedWaves;
        for (const QModelIndex& inx : mContextIndexList)
        {
            WaveData* wd = wtm->item(inx);
            if (wd) selectedWaves.append(wd);
        }
        BooleanDialog bd(selectedWaves,this);
        if (bd.exec() == QDialog::Accepted)
        {
            if (bd.hasExpression())
            {
                QString boolExpr = bd.expression();
                if (!boolExpr.isEmpty())
                {
                    wtm->insertBoolean(mContextIndexList.last(),boolExpr);
                    selectionModel()->select(mContextIndexList.last(),QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                }
            }
            else
            {
                wtm->insertBoolean(mContextIndexList.last(),selectedWaves,bd.tableValues());
                selectionModel()->select(mContextIndexList.last(),QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
    }

    void WaveTreeView::handleInsertTrigger()
    {
        if (mContextIndexList.empty()) return;
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        QList<WaveData*> trigList;
        for (const QModelIndex& inx : mContextIndexList)
        {
            WaveData* wd = wtm->item(inx);
            if (wd) trigList.append(wd);
        }
        QModelIndex insertInx = wtm->index(mContextIndexList.first().row()+1,0);
        QList<WaveData*> filterList;
        for (WaveDataBoolean* wdb : mWaveDataList->mDataBooleans.values())
            filterList.append(wdb);
        QSet<int> iwaveAdded;
        for (auto it = mWaveItemHash->constBegin(); it != mWaveItemHash->constEnd(); ++it)
        {
            if (it.key().intType()!=WaveItemIndex::Wire) continue;
            int iwave = it.key().index();
            if (iwave < 0 || iwaveAdded.contains(iwave)) continue;
            filterList.append(mWaveDataList->at(iwave));
            iwaveAdded.insert(iwave);
        }
        TriggerDialog td(trigList,filterList,this);
        if (td.exec() == QDialog::Accepted)
        {
            wtm->insertTrigger(insertInx,trigList,td.transitionToValue(),td.filterWave());
        }
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

    void WaveTreeView::handleExpand(const QModelIndex& index)
    {
        const WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        int grpId = wtm->groupId(index);
        if (grpId >= 0) mExpandedGroups.insert(grpId);

        reorder();
    }

    void WaveTreeView::handleCollapse(const QModelIndex& index)
    {
        const WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        int grpId = wtm->groupId(index);
        if (grpId >= 0) mExpandedGroups.remove(grpId);

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
        mWaveDataList->emptyTrash();
        orderRecursion(QModelIndex());

        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        WaveItemHash notPlaced = *mWaveItemHash;
        int nVisible = mItemOrder.size();
        if (nVisible != mWaveItemHash->visibleEntries())
        {
            mWaveItemHash->setVisibleEntries(nVisible);

            /*   // signal not connected
            if (verticalScrollBar()->isVisible())
                Q_EMIT numberVisibleChanged(nVisible, verticalScrollBar()->maximum(), verticalScrollBar()->value());
            else
                Q_EMIT numberVisibleChanged(nVisible, -1, -1);
            */
        }
        for (int i=0; i<nVisible; i++)
        {
            QModelIndex currentIndex = mItemOrder.at(i);
            int iwave = wtm->waveIndex(currentIndex);
            if (iwave < 0)
            {
                int id = 0;
                WaveItemIndex::IndexType tp = WaveItemIndex::Invalid;
                if (iwave == -1)
                {
                    id = wtm->groupId(currentIndex);
                    tp = WaveItemIndex::Group;
                }
                else if (iwave == -2)
                {
                    id = wtm->booleanId(currentIndex);
                    tp = WaveItemIndex::Bool;
                }
                else if (iwave == -3)
                {
                    id = wtm->triggerId(currentIndex);
                    tp = WaveItemIndex::Trig;
                }
                if (id > 0 && tp != WaveItemIndex::Invalid)
                {
                    WaveItemIndex wii(id, tp, 0);
                    WaveItem* wi = mWaveItemHash->value(wii);
                    if (!wi)
                    {
                        mWaveItemHash->dump("Crash");
                        qDebug() << "group/boolean wii not found" << id << tp;
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

            // restore expanded or collapsed group state
            int grpId = wtm->groupId(currentIndex);
            if (grpId >= 0)
            {
                if (mExpandedGroups.contains(grpId))
                    expand(currentIndex);
                else
                    collapse(currentIndex);
            }
        }

        for (WaveItem* wi : notPlaced.values())
            wi->setWaveVisible(false);

        wtm->persist();
        Q_EMIT triggerUpdateWaveItems();
    }

    void WaveTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
        QTreeView::selectionChanged(selected,deselected);
        WaveTreeModel* wtm = static_cast<WaveTreeModel*>(model());
        mWaveItemHash->setSelected();
        for (WaveItem* wi : mWaveItemHash->values())
            wi->setWaveSelected(false);
        for (const QModelIndex& inx : sortedSelection())
        {
            WaveItemIndex wii = wtm->hashIndex(inx);
            WaveItem* wi = mWaveItemHash->value(wii);
            if (wi)
            {
                wi->setWaveSelected(true);
                if (!mWaveItemHash->firstSelected())
                    mWaveItemHash->setSelected(wi);
            }
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
