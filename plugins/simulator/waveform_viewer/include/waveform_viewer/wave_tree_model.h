// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "hal_core/defines.h"
#include "netlist_simulator_controller/wave_data.h"
#include "waveform_viewer/wave_item.h"
#include <QAbstractItemModel>
#include <QList>
#include <QDir>
#include <QThread>
#include <QObject>
#include <QMetaObject>

namespace hal {
    class WaveData;
    class WaveDataList;
    class WaveDataGroup;
    class WaveGraphicsCanvas;

    class WaveDataRoot : public WaveDataGroup
    {
    public:
        WaveDataRoot(WaveDataList* wdList)
            : WaveDataGroup(wdList,0,"root") {;}
        void recalcData() override {;}
        bool moveGroupPosition(int sourceRow, int targetRow);
        void clearAll() { mGroupList.clear(); }
        void dump() const;
    };

    class WaveValueThread : public QThread
    {
        Q_OBJECT
        WaveItem* mItem;
        QDir mWorkDir;
        u64 mTpos;
        int mXpos;
        int mValue;
        bool mAbort;
    Q_SIGNALS:
        void valueThreadEnds(hal::WaveItem* item);
    private Q_SLOTS:
        void handleValueThreadFinished();
    public:
        WaveValueThread(WaveItem* item, const QString& workdir, u64 tpos, int xpos, QObject* parent = nullptr);
        void run() override;
        void abort() { mAbort = true; }
        bool wasAborted() const { return mAbort; }
    };

    class WaveTreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        enum DragCommand { None, Move, Copy };

        class ReorderRequest
        {
            WaveTreeModel* mParent;
        public:
            ReorderRequest(WaveTreeModel* p) : mParent(p) { mParent->mReorderRequestWaiting++; }
            ~ReorderRequest() {
                if (--mParent->mReorderRequestWaiting <= 0) mParent->emitReorder();
            }
        };

    private:
        WaveDataList* mWaveDataList;
        WaveItemHash* mWaveItemHash;
        WaveGraphicsCanvas* mGraphicsCanvas;
        WaveDataRoot* mRoot;
        QModelIndexList mDragIndexList;
        DragCommand mDragCommand;
        bool mDragIsGroup;
        u64 mCursorTime;
        int mCursorXpos;
        int mReorderRequestWaiting;
        int mNumberEntriesChangedEvents;
        QMap<WaveItem*,WaveValueThread*> mValueThreads;

        bool dropGroup(const QModelIndex& parentTo, int row);
        void dropRow(const QModelIndex& parentTo, int row);
        void invalidateParent(const QModelIndex& parentRow);
        void updateGroup(WaveDataGroup* grp);
        void emitReorder();
        void addOrReplaceItem(WaveData* wd, WaveItemIndex::IndexType tp, int iwave, int parentId);
        int valueAtCursor(const QModelIndex& index) const;
    Q_SIGNALS:
        void inserted(QModelIndex index);
        void triggerReorder();
        void numberEntriesChanged(int count);
        void triggerStartValueThread(WaveItem* item) const;

    public Q_SLOTS:
        void handleUpdateValueColumn();
        void handleWaveAdded(int iwave);
        void handleWaveRenamed(int iwave);
        void handleGroupRenamed(int grpId);
        void handleWaveAddedToGroup(const QVector<u32>& netIds, int grpId);
        void handleWaveRemovedFromGroup(int iwave, int grpId);
        void handleBooleanAdded(int boolId);
        void handleTriggerAdded(int trigId);
        void handleGroupAdded(int grpId);
        void handleGroupAboutToBeRemoved(WaveDataGroup* grp);
        void handleGroupUpdated(int grpId);
        void handleCursorMoved(u64 tCursor, int xpos);
        void forwardBeginResetModel();
        void forwardEndResetModel();
        //void handleValueLoaderFinished();
        void handleStartValueThread(WaveItem* item);
        void handleEndValueThread(WaveItem* item);

    public:
        WaveTreeModel(WaveDataList* wdlist, WaveItemHash* wHash, WaveGraphicsCanvas* wgc, QObject* obj=nullptr);
        bool isLeaveItem(const QModelIndex &index) const;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndexList indexes(const WaveData* wd) const;
        QModelIndexList indexes(const QSet<u32>& netIds) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QString netName(const QModelIndex& index) const;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &child) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        Qt::DropActions supportedDragActions() const override;
        bool dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &dropParent) override;
        bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
        void setDragIndexes(const QModelIndexList& indexList);

        bool insertItem(int row, const QModelIndex &parent, WaveData* wd);
        void insertExisting(int row, const QModelIndex& parent, WaveItem* wi);
        WaveItem* removeItemFromHash(int row, const QModelIndex &parent);
        void removeGroup(const QModelIndex& groupIndex);
        void insertBoolean(const QModelIndex& boolIndex, const QString& boolExpression, WaveDataBoolean* wdBool=nullptr);
        void insertBoolean(const QModelIndex& boolIndex, const QList<WaveData*>& boolWaves, const QList<int>& acceptMask);
        void insertTrigger(const QModelIndex& trigIndex, const QList<WaveData*>& trigWaves, const QList<int>& toVal,
                           WaveData* wdFilter=nullptr, WaveDataTrigger *wdTrig=nullptr);
        void insertGroup(const QModelIndex& groupIndex, WaveDataGroup *grp=nullptr);
        int waveIndex(const QModelIndex& index) const;
        int groupId(const QModelIndex& grpIndex) const;
        int booleanId(const QModelIndex& boolIndex) const;
        int triggerId(const QModelIndex& trigIndex) const;
        WaveData* item(const QModelIndex& index) const;
        WaveItemIndex hashIndex(const QModelIndex& index) const;

        void setGroupPosition(int ypos, const QModelIndex& index);
        void addWaves(const QVector<WaveData*>& wds);
        bool onlyRootItemsSelected(const QModelIndexList& selectList) const;

        u64 cursorTime() const { return mCursorTime; }
        int cursorXpos() const { return mCursorXpos; }

        QSet<int> waveDataIndexSet() const;

        static const char* sStateColor[3];
        bool persist() const;
        void restore();
        int decreaseNumberEntriesChangedEvents();
    };
}
