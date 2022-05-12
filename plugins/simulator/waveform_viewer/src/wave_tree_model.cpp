#include "waveform_viewer/wave_tree_model.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include "waveform_viewer/wave_render_engine.h"
#include "waveform_viewer/wave_graphics_canvas.h"
#include "hal_core/utilities/json_write_document.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include <QApplication>
#include <QFont>
#include <QMimeData>
#include <QAction>
#include <stdlib.h>

namespace hal {

    WaveTreeModel::WaveTreeModel(WaveDataList *wdlist, WaveItemHash *wHash, WaveGraphicsCanvas *wgc, QObject *obj)
        : QAbstractItemModel(obj), mWaveDataList(wdlist), mWaveItemHash(wHash), mGraphicsCanvas(wgc),
          mDragCommand(None), mDragIsGroup(false),
          mCursorTime(0), mCursorXpos(0), mIgnoreSignals(false),
          mReorderRequestWaiting(0)
    {

        mRoot = new WaveDataRoot(mWaveDataList);
        connect(this,&WaveTreeModel::triggerStartValueThread,this,&WaveTreeModel::handleStartValueThread,Qt::QueuedConnection);

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

    WaveData* WaveTreeModel::item(const QModelIndex& index) const
    {
        if (!index.isValid()) return nullptr;
        WaveDataGroup* grp = static_cast<WaveDataGroup*>(index.internalPointer());
        if (!grp) return mRoot;
        return grp->childAt(index.row());
    }

    WaveItemIndex WaveTreeModel::hashIndex(const QModelIndex& index) const
    {
        WaveData* wd = item(index);
        if (!wd) return WaveItemIndex();
        switch (wd->netType())
        {
        case WaveData::NetGroup:
            return WaveItemIndex(wd->id(),WaveItemIndex::Group);
        case WaveData::BooleanNet:
            return WaveItemIndex(wd->id(),WaveItemIndex::Bool);
        case WaveData::TriggerTime:
            return WaveItemIndex(wd->id(),WaveItemIndex::Trig);
        default:
            break;
        }
        int iwave = mWaveDataList->waveIndexByNetId(wd->id());
        WaveDataGroup* parentGrp = static_cast<WaveDataGroup*>(index.internalPointer());
        if (!parentGrp || parentGrp == mRoot) return WaveItemIndex(iwave,WaveItemIndex::Wire);
        return WaveItemIndex(iwave,WaveItemIndex::Wire,parentGrp->id());
    }

    QModelIndex WaveTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        WaveData* wd = item(parent);
        if (!wd) return createIndex(row,0,mRoot);
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(wd);
        if (!grp || row >= grp->size()) return QModelIndex();
        return createIndex(row,column,grp);
    }

    bool WaveTreeModel::persist() const
    {
        QDir workDir(QFileInfo(QString::fromStdString(mWaveDataList->saleaeDirectory().get_directory())).path());
        JsonWriteDocument jwd;
        JsonWriteObject& jwfv = jwd.add_object("waveform_viewer");
        JsonWriteArray& jitems = jwfv.add_array("items");
        for (WaveData* wd : mRoot->children())
        {
            JsonWriteObject& jitem = jitems.add_object();
            jitem["id"] = (int) wd->id();
            jitem["name"] = wd->name().toStdString();
            jitem["type"] = (int) wd->composedType();
            jitem.close();
        }
        jitems.close();

        if (!mGraphicsCanvas->renderEngine()->zoomHistory().isEmpty())
        {
            JsonWriteArray& jzooms = jwfv.add_array("zoom_history");
            for (const WaveZoomShift& wzs : mGraphicsCanvas->renderEngine()->zoomHistory())
            {
                JsonWriteObject& jzoom = jzooms.add_object();
                jzoom["scale"] = (double)   wzs.scale();
                jzoom["tleft"] = (uint64_t) wzs.leftTime();
                jzoom["width"] = (int)      wzs.width();
                jzoom.close();
            }
            jzooms.close();
        }
        jwfv.close();
        return jwd.serialize(workDir.absoluteFilePath("waveform_viewer.json").toStdString());
    }

    void WaveTreeModel::restore()
    {
        QDir workDir(QFileInfo(QString::fromStdString(mWaveDataList->saleaeDirectory().get_directory())).path());
        FILE* ff = fopen(workDir.absoluteFilePath("waveform_viewer.json").toUtf8().data(), "rb");
        if (!ff) return;

        char buffer[65536];
        rapidjson::FileReadStream frs(ff, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(ff);

        if (document.HasParseError() || !document.HasMember("waveform_viewer")) return;
        auto jwfv = document["waveform_viewer"].GetObject();
        ReorderRequest req(this);

        mRoot->clearAll();
        for (auto it = mWaveItemHash->begin(); it != mWaveItemHash->end(); ++it)
            delete it.value();
        mWaveItemHash->clear();

        if (jwfv.HasMember("items"))
        {
            for (auto& jitem : jwfv["items"].GetArray())
            {

                u32 id = jitem.HasMember("id") ? jitem["id"].GetUint() : 0;
                if (!id) continue;
                std::string name = jitem.HasMember("name") ? jitem["name"].GetString() : std::string();
                int composedType = jitem.HasMember("type") ? jitem["type"].GetInt() : 0;

                switch (composedType)
                {
                case SaleaeDirectoryNetEntry::Group:
                {
                    WaveDataGroup* wdGrp = mWaveDataList->mDataGroups.value(id);
                    if (wdGrp)
                    {
                        handleGroupAdded(id);
                        for (WaveData* wd : wdGrp->children())
                        {
                            int iwave = mWaveDataList->waveIndexByNetId(wd->id());
                            WaveItemIndex wii(iwave, WaveItemIndex::Wire, wdGrp->id());
                            if (!mWaveItemHash->contains(wii)) // unless already in model
                            {
                                addOrReplaceItem(wd, WaveItemIndex::Wire, iwave, wdGrp->id());
                            }
                        }
                    }
                    break;
                }
                case SaleaeDirectoryNetEntry::Boolean:
                {
                    WaveDataBoolean* wdBool = mWaveDataList->mDataBooleans.value(id);
                    if (wdBool)
                    {
                        handleBooleanAdded(id);
                    }
                    break;
                }
                case SaleaeDirectoryNetEntry::Trigger:
                {
                    WaveDataTrigger* wdTrig = mWaveDataList->mDataTrigger.value(id);
                    if (wdTrig)
                    {
                        handleTriggerAdded(id);
                    }
                    break;
                }
                default:
                {
                    int iwave = mWaveDataList->waveIndexByNetId(id);
                    if (iwave >= 0)
                    {
                        handleWaveAdded(iwave);
                    }
                }
                }
            }
        }
        if (jwfv.HasMember("zoom_history"))
        {
            QList<WaveZoomShift> zh;
            for (auto& jzoom : jwfv["zoom_history"].GetArray())
            {
                float sc = jzoom.HasMember("scale") ? jzoom["scale"].GetDouble() : 1.;
                float tl = jzoom.HasMember("tleft") ? jzoom["tleft"].GetUint64() : 0;
                float wd = jzoom.HasMember("width") ? jzoom["width"].GetInt() : 640;
                zh.append(WaveZoomShift(sc,tl,wd));
            }
            mGraphicsCanvas->renderEngine()->setZoomHistory(zh);
            mGraphicsCanvas->undoZoom();
        }
        mWaveDataList->emitTimeframeChanged();
        emitReorder();
    }

    QModelIndexList WaveTreeModel::indexes(const WaveData* wd) const
    {
        QModelIndexList retval;
        int rootRow = 0;
        for (WaveData* rootChild : mRoot->children())
        {
            if (rootChild == wd)
                retval.append(createIndex(rootRow,0,mRoot));
            else
            {
                WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(rootChild);
                if (grp)
                {
                    int groupRow = 0;
                    for (const WaveData* grpChild : grp->children())
                    {
                        if (grpChild == wd)
                            retval.append(createIndex(groupRow,0,grp));
                        groupRow++;
                    }
                }
            }
            rootRow++;
        }
        return retval;
    }

    QModelIndexList WaveTreeModel::indexes(const QSet<u32>& netIds) const
    {
        QModelIndexList retval;
        int rootRow = 0;
        for (WaveData* rootChild : mRoot->children())
        {
            WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(rootChild);
            if (grp)
            {
                int groupRow = 0;
                for (const WaveData* grpChild : grp->children())
                {
                    if (netIds.contains(grpChild->id()))
                        for (int icol=0; icol<3; icol++)
                            retval.append(createIndex(groupRow,0,grp));
                    groupRow++;
                }
            }
            else
            {
                if (netIds.contains(rootChild->id()))
                    for (int icol=0; icol<3; icol++)
                        retval.append(createIndex(rootRow,0,mRoot));
            }
            rootRow++;
        }
        return retval;
    }

    void WaveTreeModel::emitReorder()
    {
        Q_EMIT triggerReorder();
    }

    void WaveTreeModel::handleTriggerAdded(int trigId)
    {
        ReorderRequest req(this);
        if (mIgnoreSignals) return;
        WaveDataTrigger* wdTrig = mWaveDataList->mDataTrigger.value(trigId);
        if (!wdTrig) return;
        insertTrigger(createIndex(mRoot->size(),0,mRoot),QList<WaveData*>(),QList<int>(),nullptr,wdTrig);
        addOrReplaceItem(wdTrig, WaveItemIndex::Trig, wdTrig->id(), 0);
    }

    void WaveTreeModel::handleBooleanAdded(int boolId)
    {
        ReorderRequest req(this);
        if (mIgnoreSignals) return;
        WaveDataBoolean* wdBool = mWaveDataList->mDataBooleans.value(boolId);
        if (!wdBool) return;
        insertBoolean(createIndex(mRoot->size(),0,mRoot),QString(),wdBool);
        addOrReplaceItem(wdBool, WaveItemIndex::Bool, wdBool->id(), 0);
    }

    void WaveTreeModel::handleGroupAdded(int grpId)
    {
        ReorderRequest req(this);
        if (mIgnoreSignals) return;
        WaveDataGroup* grp = mWaveDataList->mDataGroups.value(grpId);
        if (!grp) return;
        insertGroup(createIndex(mRoot->size(),0,mRoot),grp);
        addOrReplaceItem(grp, WaveItemIndex::Group, grp->id(), 0);
    }

    void WaveTreeModel::addOrReplaceItem(WaveData* wd, WaveItemIndex::IndexType tp, int iwave, int parentId)
    {
        int oldCount = mWaveItemHash->size();
        WaveItem* wi = mWaveItemHash->addOrReplace(wd,tp,iwave,parentId);
        connect(wi,&WaveItem::gotCursorValue,this,&WaveTreeModel::handleUpdateValueColumn,Qt::QueuedConnection);
        int count = mWaveItemHash->size();
        if (count != oldCount) Q_EMIT numberEntriesChanged(count);
    }

    void WaveTreeModel::handleWaveAdded(int iwave)
    {
        WaveItemIndex wii(iwave, WaveItemIndex::Wire, 0);
        if (mWaveItemHash->contains(wii)) return; // already in model
        ReorderRequest req(this);
        WaveData* wd = mWaveDataList->at(iwave);
        insertItem(mRoot->size(),createIndex(0,0,nullptr),wd);
        addOrReplaceItem(wd, WaveItemIndex::Wire, iwave, 0);
    }

    void WaveTreeModel::addWaves(const QVector<WaveData*>& wds)
    {
        ReorderRequest req(this);

        beginResetModel();
        mRoot->addWaves(wds);
        for (WaveData* wd : wds)
        {
            int iwave = mWaveDataList->waveIndexByNetId(wd->id());
            addOrReplaceItem(wd, WaveItemIndex::Wire, iwave, 0);
        }
        endResetModel();

        invalidateParent(createIndex(0,0,nullptr)); // update root
    }

    void WaveTreeModel::handleWaveRenamed(int iwave)
    {
        WaveData* wd = mWaveDataList->at(iwave);
        for (QModelIndex inx : indexes(wd))
            Q_EMIT dataChanged(inx,inx);
    }

    void WaveTreeModel::handleGroupRenamed(int grpId)
    {
        WaveDataGroup* grp = mWaveDataList->mDataGroups.value(grpId);
        if (grp)
        {
            int irow = mRoot->childIndex(grp);
            if (irow >= 0)
            {
                QModelIndex inx = createIndex(irow,0,mRoot);
                Q_EMIT dataChanged(inx,inx);
            }
        }
    }

    void WaveTreeModel::handleWaveAddedToGroup(const QVector<u32> &netIds, int grpId)
    {
        WaveDataGroup* grp = mWaveDataList->mDataGroups.value(grpId);
        if (!grp) return;

        ReorderRequest req(this);
        for (u32 netId : netIds)
        {
            int iwave = mWaveDataList->waveIndexByNetId(netId);
            WaveData* wd = mWaveDataList->at(iwave);

            WaveItemIndex wii(iwave, WaveItemIndex::Wire, grpId);
            WaveItem* wi = nullptr;

            if (grpId)
            {
                // check whether waveform graph to be added to group is already present in root
                int iRootInx = mRoot->netIndex(netId);
                if (iRootInx >= 0)
                    wi = removeItemFromHash(iRootInx,createIndex(0,0,nullptr));
            }

            if (wi)
            {
                wi->setRequest(WaveItem::SetPosition);
                mWaveItemHash->insert(wii,wi);
            }
            else
            {
                wi = mWaveItemHash->value(wii);
                if (!wi)
                    addOrReplaceItem(wd,WaveItemIndex::Wire,iwave,grpId);
                else
                {
                    if (wi->wavedata() != wd)
                        wi->setWaveData(wd);
                    wi->clearRequest(WaveItem::DeleteRequest);
                    wi->clearRequest(WaveItem::DeleteAcknowledged);
                    wi->setRequest(WaveItem::SetPosition);
                }
            }

            if (mDragCommand == Move)
            {
                for (QModelIndex inx : indexes(wd))
                {
                    if (inx.internalPointer() == grp) continue;
                    if (inx.internalPointer() == mRoot) continue;
                    mWaveItemHash->dispose(removeItemFromHash(inx.row(),inx.parent()));
                }
            }
        }
        mDragCommand = None;
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
            int v = valueAtCursor(index);
            if (v == SaleaeDataTuple::sReadError)
                return QColor::fromRgb(255,255,qrand()%256);
            if (v<-1) v=-1;
            if (v>1) v=1;
            return QColor(NetlistSimulatorControllerPlugin::sSimulationSettings->color((SimulationSettings::ColorSetting) (SimulationSettings::Value0+v)));
        }
        case Qt::DisplayRole:
        {
            WaveData* wd = item(index);
            if (!wd) return QVariant();
            switch (index.column())
            {
            case 0:
                return wd->name();
            case 1:
                if (wd->id() < 1)
                    return QString();
                return QString::number(wd->id());
            case 2:
                int v = valueAtCursor(index);
                if (v == SaleaeDataTuple::sReadError)
                    return QString();
                return wd->strValue(v);
            }
        }
        default:
                break;
        }

        return QVariant();
    }

    int WaveTreeModel::waveIndex(const QModelIndex& index) const
    {
         WaveData* wd = item(index);
         if (!wd) return -1;
         if (dynamic_cast<WaveDataGroup*>(wd)) return -1;
         if (dynamic_cast<WaveDataBoolean*>(wd)) return -2;
         if (dynamic_cast<WaveDataTrigger*>(wd)) return -3;
         return mWaveDataList->waveIndexByNetId(wd->id());
    }

    int WaveTreeModel::triggerId(const QModelIndex& trigIndex) const
    {
        WaveDataTrigger* wdTrig = dynamic_cast<WaveDataTrigger*>(item(trigIndex));
        if (!wdTrig) return -1;
        return wdTrig->id();
    }

    int WaveTreeModel::booleanId(const QModelIndex& boolIndex) const
    {
        WaveDataBoolean* wdBool = dynamic_cast<WaveDataBoolean*>(item(boolIndex));
        if (!wdBool) return -1;
        return wdBool->id();
    }

    int WaveTreeModel::groupId(const QModelIndex& grpIndex) const
    {
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(item(grpIndex));
        if (!grp) return -1;
        return grp->id();
    }

    QString WaveTreeModel::netName(const QModelIndex &index) const
    {
        WaveData* wd = item(index);
        if (!wd) return QString();
        return wd->name();
    }

    int WaveTreeModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return mRoot->size();
        WaveData* wd = item(parent);
        if (!wd) return 0;
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(wd);
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
        WaveDataGroup* grp = static_cast<WaveDataGroup*>(child.internalPointer());
        if (!grp) return QModelIndex();
        if (grp == mRoot)
            return createIndex(0,0,nullptr);

        int n = mRoot->size();
        for (int i=0; i<n; i++)
        {
            if (mRoot->childAt(i) == grp)
            {
                return createIndex(i,0,mRoot);
            }
        }
        return QModelIndex();
    }

    void WaveTreeModel::forwardBeginResetModel()
    {
        beginResetModel();
    }

    void WaveTreeModel::forwardEndResetModel()
    {
        endResetModel();
    }

    Qt::DropActions WaveTreeModel::supportedDragActions() const
    {
        return Qt::MoveAction | Qt::CopyAction;
    }

    Qt::ItemFlags WaveTreeModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags retval = Qt::ItemIsDropEnabled | Qt:: ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (!index.isValid()) return retval;
        retval |= Qt::ItemIsDragEnabled;
        if (isLeaveItem(index)) return retval | Qt::ItemNeverHasChildren;
        return retval;
    }

    void WaveTreeModel::setDragIndexes(const QModelIndexList& indexList) {
        mDragIsGroup = true;
        mDragIndexList.clear();


        for (const QModelIndex& inx : indexList)
        {
            if (isLeaveItem(inx))
            {
                mDragIsGroup = false;
                break;
            }
        }

        if (mDragIsGroup)
        {
            mDragIndexList = indexList;
        }
        else
        {
            mDragIndexList.clear();
            for (const QModelIndex& inx : indexList)
                if (isLeaveItem(inx))
                    mDragIndexList.append(inx);
        }
        mDragCommand = Move;
    }

    bool WaveTreeModel::dropGroup(const QModelIndex &parentTo, int row)
    {
        ReorderRequest req(this);
        if (row < 0) row = parentTo.row();
        bool couldDrop = false;
        beginResetModel();
        for (const QModelIndex& inx : mDragIndexList)
        {
            if (inx.column()) continue;
            if (inx.internalPointer() != mRoot) continue;
            if (!mRoot->moveGroupPosition(inx.row(),row)) continue;
            couldDrop = true;
        }
        endResetModel();
        return couldDrop;
    }

    void WaveTreeModel::dropRow(const QModelIndex& parentTo, int row)
    {
        ReorderRequest req(this);
        int targetRow = row;

        WaveDataGroup* targetGrp = static_cast<WaveDataGroup*>(isLeaveItem(parentTo)
                                                               ? parentTo.internalPointer()
                                                               : item(parentTo));

        QList<WaveItem*> wiTempList;

        for (QModelIndex inxSource : mDragIndexList)
        {
            if (inxSource.column()) continue;
            QModelIndex sourceParentIndex = inxSource.parent();
            WaveDataGroup* sourceGrp = static_cast<WaveDataGroup*>(inxSource.internalPointer());
            if (targetRow > 0 && sourceGrp == targetGrp && inxSource.row() < targetRow)
                --targetRow; // compensate position for removed items
            WaveItem* wi = removeItemFromHash(inxSource.row(),sourceParentIndex);
            if (!wi) continue;
            wiTempList.append(wi);
        }

        QModelIndex targetIndex = createIndex(targetRow,0,targetGrp);
        QModelIndex targetIndexParent = targetIndex.parent();

        for (WaveItem* wi : wiTempList)
            insertExisting(targetRow,targetIndexParent,wi);
        Q_EMIT inserted(index(targetRow,0,targetIndexParent));
    }

    void WaveTreeModel::handleCursorMoved(double tCursor, int xpos)
    {
        if (tCursor == mCursorTime && xpos == mCursorXpos) return;
        if (!mValueThreads.isEmpty())
        {
            for (WaveValueThread* wvt : mValueThreads.values())
                wvt->abort();
        }
        mCursorTime = tCursor;
        mCursorXpos = xpos;
        handleUpdateValueColumn();
    }

    void WaveTreeModel::handleEndValueThread(WaveItem* item)
    {
        bool workDone = false;
        auto it = mValueThreads.find(item);
        if (it != mValueThreads.end())
        {
            workDone = !it.value()->wasAborted();
            mValueThreads.erase(it);
        }
        if (workDone)
            handleUpdateValueColumn();
    }

    void WaveTreeModel::handleUpdateValueColumn()
    {
        QModelIndex i0 = createIndex(0,2,mRoot);
        QModelIndex i1 = createIndex(mRoot->size()-1,2,mRoot);

        Q_EMIT dataChanged(i0,i1);
        for (WaveDataGroup* grp : mWaveDataList->mDataGroups.values())
        {
            int n = grp->size();
            if (!n) continue;
            i0 = createIndex(0,2,grp);
            i1 = createIndex(n-1,2,grp);
            Q_EMIT dataChanged(i0,i1);
        }
    }

    bool WaveTreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &dropParent)
    {
        Q_UNUSED(action);
        Q_UNUSED(column);
        if (!dropParent.isValid()) return false;
        if (!mimeData->formats().contains("application/x-qabstractitemmodeldatalist")) return false;
        if (mDragIsGroup) return dropGroup(dropParent,row);

        // drop on wave item, insert behind
        if (isLeaveItem(dropParent))
        {
            WaveData* wd = item(dropParent);
            WaveDataGroup* grp = static_cast<WaveDataGroup*>(dropParent.internalPointer());
            int n=grp->size();
            for (int i=0; i<n; i++)
                if (grp->childAt(i) == wd)
                {
                    dropRow(dropParent.parent(),i+1);
                    return true;
                }
            return false;
        }

        // drop on group item
        if (dropParent.internalPointer() == mRoot)
        {
            dropRow(dropParent,0);
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
        if (parent.internalPointer() == mRoot) return true;
        if (!mDragIsGroup && isLeaveItem(parent)) return true;
        return false;
    }

    bool WaveTreeModel::isLeaveItem(const QModelIndex &index) const
    {
        WaveData* wd = item(index);
        if (!wd) return false;
        return dynamic_cast<WaveDataGroup*>(wd)==nullptr;
    }

    //TODO : unify insertExisting und insertItem
    void WaveTreeModel::insertExisting(int row, const QModelIndex& parent, WaveItem* wi)
    {
        if (!parent.isValid()) return;
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(item(parent));
        if (!grp) return;
        if (row < 0) row = 0;
        if (row > grp->size()) row = grp->size();

        const WaveData* wdConst = wi->wavedata();
        WaveData* wd = nullptr;
        int iwave = -1;
        WaveItemIndex::IndexType inxTp = WaveItemIndex::Invalid;
        int grpId = grp->id();

        switch (wdConst->netType())
        {
        case WaveData::NetGroup:
            inxTp = WaveItemIndex::Group;
            iwave = wdConst->id();
            grpId = 0;
            wd = mWaveDataList->mDataGroups.value(iwave);
            break;
        case WaveData::BooleanNet:
            inxTp = WaveItemIndex::Bool;
            iwave = wdConst->id();
            grpId = 0;
            wd = mWaveDataList->mDataBooleans.value(iwave);
            break;
        case WaveData::TriggerTime:
            inxTp = WaveItemIndex::Trig;
            iwave = wdConst->id();
            grpId = 0;
            wd = mWaveDataList->mDataTrigger.value(iwave);
            break;
        default:
            inxTp = WaveItemIndex::Wire;
            iwave = mWaveDataList->waveIndexByNetId(wi->wavedata()->id());
            wd = mWaveDataList->at(iwave);
            break;
        }
        if (iwave < 0 || !wd) return;

        beginResetModel();
        grp->insert(row,wd);
        grp->recalcData();
        endResetModel();

        WaveItemIndex wii(iwave, inxTp, grpId);
        mWaveItemHash->insert(wii,wi);
        invalidateParent(parent);
    }

    bool WaveTreeModel::insertItem(int row, const QModelIndex &parent, WaveData *wd)
    {
        if (!parent.isValid()) return false;
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(item(parent));
        if (!grp) return false;
        if (row < 0) row = 0;
        if (row > grp->size()) row = grp->size();

        beginResetModel();
        grp->insert(row,wd);
        grp->recalcData();
        endResetModel();

        switch (wd->netType())
        {
        case WaveData::NetGroup:
        {
            WaveDataGroup* wdGrp = static_cast<WaveDataGroup*>(wd);
            addOrReplaceItem(wd, WaveItemIndex::Group, wdGrp->id(), 0);
            break;
        }
        case WaveData::BooleanNet:
        {
            WaveDataBoolean* wdBool = static_cast<WaveDataBoolean*>(wd);
            wdBool->recalcData();
            addOrReplaceItem(wd, WaveItemIndex::Bool, wdBool->id(),0);
            break;
        }
        case WaveData::TriggerTime:
        {
            WaveDataTrigger* wdTrig = static_cast<WaveDataTrigger*>(wd);
            wdTrig->recalcData();
            addOrReplaceItem(wd, WaveItemIndex::Trig, wdTrig->id(),0);
            break;
        }
        default:
            handleWaveAddedToGroup({wd->id()},grp->id());
            break;
        }

        invalidateParent(parent);
        return true;
    }

    void WaveTreeModel::invalidateParent(const QModelIndex &parentRow)
    {
        QModelIndex parentValueInx = createIndex(parentRow.row(),2,parentRow.internalPointer());
        Q_EMIT dataChanged(parentValueInx,parentValueInx);
    }

    void WaveTreeModel::handleStartValueThread(WaveItem* item)
    {
        if (!mValueThreads.contains(item) && !item->isLoading() && !item->isThreadBusy())
        {
            QString workdir = QString::fromStdString(mWaveDataList->saleaeDirectory().get_directory());
            WaveValueThread* wvt = new WaveValueThread(item,workdir,mCursorTime,mCursorXpos);
            connect(wvt,&WaveValueThread::valueThreadEnds,this,&WaveTreeModel::handleEndValueThread);
            wvt->start();
            mValueThreads.insert(item,wvt);
        }
    }

    int WaveTreeModel::valueAtCursor(const QModelIndex& index) const
    {
        WaveData* wd = item(index);
        WaveItemIndex wii;
        switch (wd->netType())
        {
        case WaveData::NetGroup:
            wii = WaveItemIndex(wd->id(), WaveItemIndex::Group);
            break;
        case WaveData::BooleanNet:
            wii = WaveItemIndex(wd->id(), WaveItemIndex::Bool);
            break;
        case WaveData::TriggerTime:
            wii = WaveItemIndex(wd->id(), WaveItemIndex::Trig);
            break;
        default:
            WaveDataGroup* grp = static_cast<WaveDataGroup*>(index.internalPointer());
            int iwave = mWaveDataList->waveIndexByNetId(wd->id());
            wii = WaveItemIndex(iwave, WaveItemIndex::Wire, grp->id());
            break;
        }

        WaveItem* wi = mWaveItemHash->value(wii);
        if (!wi) return SaleaeDataTuple::sReadError;
        int retval = wi->cursorValue(mCursorTime,mCursorXpos);
        if (retval == SaleaeDataTuple::sReadError)
        {
            Q_EMIT triggerStartValueThread(wi);
        }
        return retval;
    }


    WaveItem *WaveTreeModel::removeItemFromHash(int row, const QModelIndex &parent)
    {
        if (!parent.isValid()) return nullptr;
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(item(parent));
        if (!grp) return nullptr;

        ReorderRequest req(this);
        beginRemoveRows(parent,row,row);
        WaveData* wd = grp->removeAt(row);
        WaveItemIndex::IndexType inxTp = WaveItemIndex::Invalid;
        endRemoveRows();
        invalidateParent(parent);
        grp->recalcData();
        int iwave = mWaveDataList->waveIndexByNetId(wd->id());
        switch (wd->netType())
        {
        case WaveData::TriggerTime:
            inxTp = WaveItemIndex::Trig;
            iwave = wd->id();
            break;
        case WaveData::BooleanNet:
            inxTp = WaveItemIndex::Bool;
            iwave = wd->id();
            break;
        default:
            inxTp = WaveItemIndex::Wire;
            break;
        }
        WaveItemIndex wii(iwave, inxTp, grp->id());
        auto it = mWaveItemHash->find(wii);
        WaveItem* retval = nullptr;
        if (it != mWaveItemHash->end())
        {
            retval = it.value();
            mWaveItemHash->erase(it);
        }
        return retval;
    }

    void WaveTreeModel::removeGroup(const QModelIndex& groupIndex)
    {
        if (groupIndex.internalPointer() != mRoot) return;

        ReorderRequest req(this);
        int irow = groupIndex.row();
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(mRoot->childAt(irow));
        mWaveDataList->removeGroup(grp->id());
        WaveItemIndex wii(grp->id(), WaveItemIndex::Group);
        auto it = mWaveItemHash->find(wii);
        if (it != mWaveItemHash->end())
        {
            WaveItem* wi = it.value();
            mWaveItemHash->erase(it);
            if (wi) mWaveItemHash->dispose(wi);
        }
    }

    void WaveTreeModel::handleGroupUpdated(int grpId)
    {
        WaveItem* wi = mWaveItemHash->value(WaveItemIndex(grpId,WaveItemIndex::Group));
        if (wi) wi->setRequest(WaveItem::DataChanged);
    }

    void WaveTreeModel::handleWaveRemovedFromGroup(int iwave, int grpId)
    {
        WaveItemIndex wii(iwave, WaveItemIndex::Wire, grpId);
        auto it = mWaveItemHash->find(wii);
        if (it != mWaveItemHash->end())
        {
            beginResetModel();
            WaveItem* wi = it.value();
            mWaveItemHash->erase(it);
            if (wi) mWaveItemHash->dispose(wi);
            endResetModel();
        }
    }

    void WaveTreeModel::handleGroupAboutToBeRemoved(WaveDataGroup* grp)
    {
        if (!grp) return;
        int irow = mRoot->childIndex(grp);
        if (irow<0) return;

        ReorderRequest req(this);
        beginResetModel();
        mRoot->removeAt(irow);
        if (!grp->isEmpty())
        {
            int n = grp->size() - 1;
            for (int i=n; i>=0; i--)
            {
                WaveData* wd = grp->childAt(i);
                int iwave = mWaveDataList->waveIndexByNetId(wd->id());
                // remove wave items from group
                auto it = mWaveItemHash->find(WaveItemIndex(iwave,WaveItemIndex::Wire,grp->id()));
                if (it != mWaveItemHash->end())
                {
                    WaveItem* wi = it.value();
                    mWaveItemHash->erase(it);
                    if (wi) mWaveItemHash->dispose(wi);
                }
            }
        }
        endResetModel();
        WaveItem* wi = mWaveItemHash->value(WaveItemIndex(grp->id(),WaveItemIndex::Group));
        if (wi) wi->setRequest(WaveItem::DeleteRequest);
    }

    void WaveTreeModel::insertTrigger(const QModelIndex& trigIndex, const QList<WaveData*>& trigWaves, const QList<int>& toVal, WaveData *wdFilter, WaveDataTrigger *wdTrig)
    {
        if (trigIndex.internalPointer() != mRoot) return;

        ReorderRequest req(this);
        mIgnoreSignals = true;
        if (!wdTrig)
        {
            wdTrig = new WaveDataTrigger(mWaveDataList,trigWaves,toVal);
            if (wdFilter) wdTrig->set_filter_wave(wdFilter);
        }
        beginResetModel();
        insertItem(trigIndex.row(),trigIndex.parent(),wdTrig);
        endResetModel();
        mIgnoreSignals = false;
        wdTrig->recalcData();
    }

    void WaveTreeModel::insertBoolean(const QModelIndex& boolIndex, const QString &boolExpression, WaveDataBoolean *wdBool)
    {
        if (boolIndex.internalPointer() != mRoot) return;

        ReorderRequest req(this);
        mIgnoreSignals = true;
        if (!wdBool) wdBool = new WaveDataBoolean(mWaveDataList,boolExpression);
        beginResetModel();
        insertItem(boolIndex.row(),boolIndex.parent(),wdBool);
        endResetModel();
        mIgnoreSignals = false;
        wdBool->recalcData();
    }

    void WaveTreeModel::insertBoolean(const QModelIndex& boolIndex, const QList<WaveData *> &boolWaves, const QList<int>& acceptMask)
    {
        if (boolIndex.internalPointer() != mRoot) return;

        ReorderRequest req(this);
        mIgnoreSignals = true;
        WaveDataBoolean* wdBool = new WaveDataBoolean(mWaveDataList,boolWaves,acceptMask);
        beginResetModel();
        insertItem(boolIndex.row(),boolIndex.parent(),wdBool);
        endResetModel();
        mIgnoreSignals = false;
        wdBool->recalcData();
    }

    void WaveTreeModel::insertGroup(const QModelIndex &groupIndex, WaveDataGroup* grp)
    {
        if (groupIndex.internalPointer() != mRoot) return;

        ReorderRequest req(this);
        mIgnoreSignals = true;
        if (!grp) grp = new WaveDataGroup(mWaveDataList);
        beginResetModel();
        insertItem(groupIndex.row(),groupIndex.parent(),grp);
        endResetModel();
        mIgnoreSignals = false;
        grp->recalcData();
    }

    QSet<int> WaveTreeModel::waveDataIndexSet() const
    {
        QSet<int> retval;
        for (const WaveData* wd : mRoot->children())
        {
            const WaveDataGroup* grp = dynamic_cast<const WaveDataGroup*>(wd);
            if (grp)
            {
                for (const WaveData* wdGrp : grp->children())
                {
                    int iwave = mWaveDataList->waveIndexByNetId(wdGrp->id());
                    if (iwave >= 0) retval.insert(iwave);
                }
            }
            else
            {
                int iwave = mWaveDataList->waveIndexByNetId(wd->id());
                if (iwave >= 0) retval.insert(iwave);
            }
        }
        return retval;
    }

    bool WaveTreeModel::onlyRootItemsSelected(const QModelIndexList& selectList) const
    {
        for (const QModelIndex& inx : selectList)
            if (inx.internalPointer() != mRoot)
                return false;
        return true;
    }

    // ---- WaveDataRoot

    bool WaveDataRoot::moveGroupPosition(int sourceRow, int targetRow)
    {
        if (targetRow==sourceRow) return false;
        WaveDataGroup* grp = dynamic_cast<WaveDataGroup*>(mGroupList.at(sourceRow));
        if (!grp) return false;
        mGroupList.removeAt(sourceRow);
        mGroupList.insert( targetRow, grp);
        restoreIndex();
        return true;
    }

    void WaveDataRoot::dump() const
    {
        QTextStream xout(stderr, QIODevice::WriteOnly);
        const char* ctype = ".GBT   ";
        xout.setFieldWidth(5);
        xout << "---\n";
        for (WaveData* wd : mGroupList)
        {
            xout << wd->id() << ctype[wd->composedType()] << " <" << wd->name() << ">\n";
        }
    }

    //------------------------

    /*
    class WaveValueThread : public QThread
    {
        Q_OBJECT
        WaveItem* mItem;
        QDir mWorkDir;
        float mTposition;
        int mValue;
    public:
        WaveValueThread(WaveItem* parentItem, const QString& workdir, float tpos);
        void run() override;
    };
    */


        WaveValueThread::WaveValueThread(WaveItem* item, const QString& workdir, float tpos, int xpos, QObject *parent)
            : QThread(parent), mItem(item), mWorkDir(workdir), mTpos(tpos), mXpos(xpos), mAbort(false)
        {
            connect(this,&QThread::finished,this,&WaveValueThread::handleValueThreadFinished);
        }

        void WaveValueThread::run()
        {
            int fileIndex = mItem->wavedata()->fileIndex();
            SaleaeInputFile sif(mWorkDir.absoluteFilePath(QString("digital_%1.bin").arg(fileIndex)).toStdString());
            if (sif.good())
            {
                int val = sif.get_int_value(mTpos);
                if (!mAbort)
                    mItem->mPainted.setCursorValue(mTpos,mXpos,val);
            }
        }

        void WaveValueThread::handleValueThreadFinished()
        {
            Q_EMIT valueThreadEnds(mItem);
            deleteLater();
        }

    //------------------------

}
