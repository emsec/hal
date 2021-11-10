#include "vcd_viewer/wave_tree_model.h"
#include "vcd_viewer/volatile_wave_data.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QApplication>
#include <QFont>
#include <QMimeData>
#include <QDebug>
#include <QAction>
#include <stdlib.h>

namespace hal {
    WaveTreeItem::WaveTreeItem(u32 id_, const QString& nam, int iwave)
        : mId(id_), mName(nam), mWaveIndex(iwave)
    {;}

    /*
    QVariant WaveTreeItem::data(int icol) const
    {
        switch(icol)
        {
        case 0: return mName;
        case 1: return mId;
        case 2: return QString::number(value(),16);
        }
        return QVariant();
    }


    int WaveTreeGroup::value() const
    {
        if (size()>30) return -1;
        int retval = 0;
        int mask = 1;
        for (auto it = constBegin(); it!= constEnd(); ++it)
        {
            int v = (*it)->value();
            if (v<0) return v;
            if (v) retval |= mask;
            mask <<= 1;
        }
        return retval;
    }
    */

    u32 WaveTreeGroup::sMaxGroupId = 0;

    WaveTreeGroup::WaveTreeGroup(const QString& nam)
        : WaveTreeItem(0,nam,-1)
    {
        mGroupId = ++sMaxGroupId;
        if (nam.isEmpty())
            setName(QString("group_%1").arg(mGroupId));
    }

    const char* WaveTreeModel::sStateColor[3] = {"#707071", "#102080", "#802010"};

    WaveTreeModel::WaveTreeModel(WaveDataList *wdlist, VolatileWaveData *wdVol, QObject *obj)
        : QAbstractItemModel(obj), mWaveDataList(wdlist), mVolatileWaveData(wdVol),
          mValueBase(16), mCursorPosition(0)
    {

        mRoot = new WaveTreeGroup;

        /*
        for (int i=100; i<104; i++)
            mRoot->append(new WaveTreeItem( i, QString("item%1").arg(i) ));

        for (int i=0; i<3; i++)
        {
            WaveTreeGroup* wtg = new WaveTreeGroup(i+1, QString("group%1").arg(i+1));
            for (int j=0; j<4; j++)
                wtg->append(new WaveTreeItem( (i+1)*10+j, QString("gitem%1").arg((i+1)*10+j) ));
            mRoot->append(wtg);
        }
        for (int i=104; i<108; i++)
            mRoot->append(new WaveTreeItem( i, QString("item%1").arg(i) ));
            */
    }

    WaveTreeItem* WaveTreeModel::item(const QModelIndex& index) const
    {
        if (!index.isValid()) return nullptr;
        WaveTreeGroup* grp = static_cast<WaveTreeGroup*>(index.internalPointer());
        if (!grp) return mRoot;
        return grp->at(index.row());
    }

    QModelIndex WaveTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        WaveTreeItem* wti = item(parent);
        if (!wti) return createIndex(row,0,mRoot);
        WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(wti);
        if (!grp || row >= grp->size()) return QModelIndex();
        return createIndex(row,column,grp);
    }

    void WaveTreeModel::handleWaveAdded(int iwave)
    {
        WaveData* wd = mWaveDataList->at(iwave);
        WaveTreeItem* wti = new WaveTreeItem(wd->id(),wd->name(),iwave);
        insertItem(mRoot->size(),createIndex(0,0,nullptr),wti);
    }

    QVariant WaveTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal) return QVariant();
        if (role == Qt::SizeHintRole)
            return QSize(400,28);
        if (role != Qt::DisplayRole) return QVariant();
        switch (section)
        {
        case 0: return "Name";
        case 1: return "ID";
        case 2: return "Value";
        }
        return QVariant();
    }

    QVariant WaveTreeModel::data(const QModelIndex &index, int role) const
    {
        switch (role)
        {
        case Qt::FontRole:
        {
            QFont font = qApp->font();
            if (!isLeaveItem(index) || index.column() == 2)
            {
                font.setPixelSize(14);
                font.setBold(true);
            }
            return font;
        }
        case Qt::SizeHintRole:
            return QSize(400,28);
        case Qt::TextAlignmentRole:
            if (!index.column())
                return Qt::AlignLeft;
            return Qt::AlignRight;
        case Qt::ForegroundRole:
            if (index.column() != 2) return QVariant();
            return QColor(Qt::white);
        case Qt::BackgroundRole:
        {
            if (index.column() != 2) return QVariant();
            WaveTreeItem* wti = item(index);
            int v = itemValue(wti);
            if (v<-1) v=-1;
            if (v>1) v=1;
            return QColor(sStateColor[v+1]);
        }
        case Qt::DisplayRole:
        {
            WaveTreeItem* wti = item(index);
            if (!wti) return QVariant();
            switch (index.column())
            {
            case 0:
                return wti->name();
            case 1:
                if (wti->id() < 1)
                    return QString();
                return QString::number(wti->id());
            case 2:
            {
                int v = itemValue(wti);
                if (v<0) return "X";
                QString retval = QString::number(v,mValueBase);
                if (v>1) switch (mValueBase) {
                case 2:
                    retval.prepend("0b");
                    break;
                case 16:
                    retval.prepend("0x");
                    break;
                }
                return retval;
            }}
        }
        default:
                break;
        }

        return QVariant();
    }

    int WaveTreeModel::itemValue(const WaveTreeItem* wti) const
    {
        WaveData* wd = nullptr;
        int iwave = wti->waveIndex();
        if (iwave >= 0)
            wd = mWaveDataList->at(iwave);
        else
        {
            const WaveTreeGroup* grp = dynamic_cast<const WaveTreeGroup*>(wti);
            if (grp)
            {
                wd = mVolatileWaveData->waveData(grp->groupId());
            }
        }
        if (!wd || wd->isEmpty()) return -1;
        return wd->intValue(mCursorPosition);
    }

    int WaveTreeModel::waveIndex(const QModelIndex& index) const
    {
         WaveTreeItem* wti = item(index);
         if (!wti) return -1;
         return wti->waveIndex();
    }

    int WaveTreeModel::groupId(const QModelIndex& grpIndex) const
    {
        WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(item(grpIndex));
        if (!grp) return -1;
        return grp->groupId();
    }

    QString WaveTreeModel::netName(const QModelIndex &index) const
    {
        WaveTreeItem* wti = item(index);
        if (!wti) return QString();
        return wti->name();
    }

    int WaveTreeModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return mRoot->size();
        WaveTreeItem* wti = item(parent);
        if (!wti) return 0;
        WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(wti);
        if (!grp) return 0;
        return grp->size();
    }

    int WaveTreeModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QModelIndex WaveTreeModel::parent(const QModelIndex &child) const
    {
        if (!child.isValid()) return QModelIndex();
        WaveTreeGroup* grp = static_cast<WaveTreeGroup*>(child.internalPointer());
        if (!grp) return QModelIndex();
        if (grp == mRoot)
            return createIndex(0,0,nullptr);

        int n = mRoot->size();
        for (int i=0; i<n; i++)
        {
            if (mRoot->at(i) == grp)
            {
                return createIndex(i,0,mRoot);
            }
        }
        return QModelIndex();
    }

    QSet<int> WaveTreeModel::waveDataIndexSet() const
    {
        QSet<int> retval;
        for (const WaveTreeItem* wti : *mRoot)
        {
            const WaveTreeGroup* grp = dynamic_cast<const WaveTreeGroup*>(wti);
            if (grp)
            {
                for (const WaveTreeItem* wtii : *grp )
                    retval.insert(wtii->waveIndex());
            }
            else
                retval.insert(wti->waveIndex());
        }
        return retval;
    }

    QList<QModelIndex> WaveTreeModel::indexByNetIds(const QSet<u32>& netIds)
    {
        QList<QModelIndex> retval;
        for (int i=0; i<mRoot->size(); i++)
        {
            const WaveTreeItem* wti = mRoot->at(i);
            const WaveTreeGroup* grp = dynamic_cast<const WaveTreeGroup*>(wti);
            if (grp)
            {
                for (int j=0; j<grp->size(); j++)
                {
                    const WaveTreeItem* wtii = grp->at(j);
                    if (netIds.contains(wtii->id()))
                        retval.append(index(j,0,index(i,0)));
                }
            }
            else
            {
                if (netIds.contains(wti->id()))
                retval.append(index(i,0));
            }
        }
        return retval;
    }


    Qt::DropActions WaveTreeModel::supportedDragActions() const
    {
        return Qt::MoveAction | Qt::CopyAction;
    }

    Qt::ItemFlags WaveTreeModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags retval = Qt::ItemIsDropEnabled | Qt:: ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (!index.isValid()) return retval;
        if (isLeaveItem(index)) return retval | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren;
        return retval;
    }

    void WaveTreeModel::dropRow(const QModelIndex& parentTo, int row)
    {
        QModelIndex parentFrom = mDragIndex.parent();
        int targetRow = (parentTo == parentFrom && mDragIndex.row() < row) ? row-1 : row;
        QModelIndex targetIndex = parentTo;
        if (parentTo.internalPointer() == mDragIndex.internalPointer() && mDragIndex.row() < parentTo.row())
            targetIndex = createIndex(parentTo.row()-1,0,parentTo.internalPointer());
        WaveTreeItem* itm = removeItem(mDragIndex.row(),parentFrom);
        if (!itm) return;
        insertItem(targetRow,targetIndex,itm);
        Q_EMIT dropped();
    }

    void WaveTreeModel::handleCursorMoved(float xpos)
    {
        mCursorPosition = xpos;
        QModelIndex i0 = createIndex(0,2,mRoot);
        QModelIndex i1 = createIndex(mRoot->size()-1,2,mRoot);
        Q_EMIT dataChanged(i0,i1);
    }

    void WaveTreeModel::handleSetValueFormat()
    {
        QAction* act = static_cast<QAction*>(sender());
        mValueBase = act->data().toInt();
        QModelIndex i0 = createIndex(0,2,mRoot);
        QModelIndex i1 = createIndex(mRoot->size()-1,2,mRoot);
        Q_EMIT dataChanged(i0,i1);
    }

    bool WaveTreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &dropParent)
    {
        Q_UNUSED(action);
        Q_UNUSED(column);
        if (!dropParent.isValid()) return false;
        if (!mimeData->formats().contains("application/x-qabstractitemmodeldatalist")) return false;
        if (isLeaveItem(dropParent))
        {
            WaveTreeItem* itm = item(dropParent);
            WaveTreeGroup* grp = static_cast<WaveTreeGroup*>(dropParent.internalPointer());
            int n=grp->size();
            for (int i=0; i<n; i++)
                if (grp->at(i) == itm)
                {
                    dropRow(dropParent.parent(),i+1);
                    return true;
                }
            return false;
        }
        if (dropParent.internalPointer() == mRoot)
        {
            dropRow(dropParent,row);
            return true;
        }
        return false;
    }

    bool WaveTreeModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
    {
        Q_UNUSED(action);
        Q_UNUSED(row);
        Q_UNUSED(column);
        if (!parent.isValid()) return false;
        if (!data->formats().contains("application/x-qabstractitemmodeldatalist")) return false;
        if (isLeaveItem(parent)) return true;
        if (parent.internalPointer() == mRoot) return true;
        return false;
    }

    bool WaveTreeModel::isLeaveItem(const QModelIndex &index) const
    {
        WaveTreeItem*  itm = item(index);
        if (!itm) return false;
        return dynamic_cast<WaveTreeGroup*>(itm)==nullptr;
    }

    bool WaveTreeModel::insertItem(int row, const QModelIndex &parent, WaveTreeItem* itm)
    {
        if (!parent.isValid()) return false;
        WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(item(parent));
        if (!grp) return false;
        if (row < 0) row = 0;
        beginInsertRows(parent,row,row);
        if (row >= grp->size())
        {
            row = grp->size();
            grp->append(itm);
        }
        else
            grp->insert(row,itm);
        endInsertRows();
        Q_EMIT inserted(index(row,0,parent));
//        Q_EMIT indexInserted(itm->waveIndex(),false);
        invalidateParent(parent);
        updateVolatile(grp);
        return true;
    }

    void WaveTreeModel::updateVolatile(WaveTreeGroup *grp)
    {
        if (!grp || grp == mRoot) return;

        WaveData* wd;

        if (grp->isEmpty())
            wd = new WaveData(grp->groupId(),grp->name());
        else
        {
            QList<WaveData*> wdList;
            for (WaveTreeItem* wti : *grp)
            {
                if (wti->waveIndex()<0) return;
                wdList.append(mWaveDataList->at(wti->waveIndex()));
            }
            wd = new WaveData(grp->groupId(),grp->name(),wdList);
        }
        mVolatileWaveData->addOrReplace(grp->groupId(),wd);
    }

    void WaveTreeModel::invalidateParent(const QModelIndex &parentRow)
    {
        QModelIndex parentValueInx = createIndex(parentRow.row(),2,parentRow.internalPointer());
        Q_EMIT dataChanged(parentValueInx,parentValueInx);
    }

    WaveTreeItem* WaveTreeModel::removeItem(int row, const QModelIndex &parent)
    {
        if (!parent.isValid()) return nullptr;
        WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(item(parent));
        if (!grp) return nullptr;
        beginRemoveRows(parent,row,row);
        WaveTreeItem* retval = grp->at(row);
        grp->removeAt(row);
        endRemoveRows();
        invalidateParent(parent);
        updateVolatile(grp);
        Q_EMIT indexRemoved(retval->waveIndex(),false);
        return retval;
    }

    void WaveTreeModel::removeGroup(const QModelIndex& groupIndex)
    {
        if (groupIndex.internalPointer() != mRoot) return;
        int irow = groupIndex.row();
        WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(mRoot->at(irow));
        if (!grp) return;
        beginResetModel();
        mRoot->removeAt(irow);
        if (!grp->isEmpty())
        {
            auto it = grp->end();
            while (it != grp->begin())
            {
                --it;
                mRoot->insert(irow,*it);
            }
        }
        endResetModel();
        Q_EMIT indexRemoved(grp->groupId(),true);
        delete grp;
    }

    void WaveTreeModel::insertGroup(const QModelIndex &groupIndex)
    {
         if (groupIndex.internalPointer() != mRoot) return;
         WaveTreeGroup* grp = new WaveTreeGroup;
         beginResetModel();
         insertItem(groupIndex.row(),groupIndex.parent(),grp);
         endResetModel();
         updateVolatile(grp);
    }

    void WaveTreeModel::setVolatilePosition(int ypos, const QModelIndex& index)
    {
         WaveTreeGroup* grp = dynamic_cast<WaveTreeGroup*>(item(index));
         u32 gid = grp->groupId();
         if (mVolatileWaveData->hasGroup(gid))
             mVolatileWaveData->setYposition(gid,ypos);
    }
}
