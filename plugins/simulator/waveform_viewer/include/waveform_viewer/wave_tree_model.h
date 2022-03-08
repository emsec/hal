#pragma once
#include "hal_core/defines.h"
#include "netlist_simulator_controller/wave_data.h"
#include "waveform_viewer/wave_item.h"
#include <QAbstractItemModel>
#include <QList>
#include <QDir>
#include <QThread>

namespace hal {
    class WaveData;
    class WaveDataList;
    class WaveDataGroup;

    class WaveDataRoot : public WaveDataGroup
    {
    public:
        WaveDataRoot(WaveDataList* wdList)
            : WaveDataGroup(wdList,0,"root") {;}
        void recalcData() override {;}
        bool moveGroupPosition(int sourceRow, int targetRow);
    };

    class WaveValueThread : public QThread
    {
        Q_OBJECT
        WaveItem* mItem;
        QDir mWorkDir;
        float mTpos;
        int mXpos;
        int mValue;
        bool mAbort;
    Q_SIGNALS:
        void gotValue(WaveItem* item);
    private Q_SLOTS:
        void handleValueThreadFinished();
    public:
        WaveValueThread(WaveItem* item, const QString& workdir, float tpos, int xpos, QObject* parent = nullptr);
        void run() override;
        void abort() { mAbort = true; }
    };

    class WaveTreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        enum DragCommand { None, Move, Copy };
    private:
        WaveDataList* mWaveDataList;
        WaveItemHash* mWaveItemHash;
        WaveDataRoot* mRoot;
        QModelIndexList mDragIndexList;
        DragCommand mDragCommand;
        bool mDragIsGroup;
        float mCursorTime;
        int mCursorXpos;
        bool mIgnoreSignals;
        int mReorderRequestWaiting;
        QMap<WaveItem*,WaveValueThread*> mValueThreads;

        class ReorderRequest
        {
            WaveTreeModel* mParent;
        public:
            ReorderRequest(WaveTreeModel* p) : mParent(p) { mParent->mReorderRequestWaiting++; }
            ~ReorderRequest() {
                if (--mParent->mReorderRequestWaiting <= 0) mParent->emitReorder();
            }
        };

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
        void handleNameUpdated(int iwave);
        void handleWaveAddedToGroup(const QVector<u32>& netIds, int grpId);
        void handleGroupAdded(int grpId);
        void handleGroupAboutToBeRemoved(WaveDataGroup* grp);
        void handleGroupUpdated(int grpId);
        void handleCursorMoved(float tCursor, int xpos);
        void forwardBeginResetModel();
        void forwardEndResetModel();
        void handleValueLoaderFinished();
        void handleStartValueThread(WaveItem* item);
        void handleEndValueThread(WaveItem* item);

    public:
        WaveTreeModel(WaveDataList* wdlist, WaveItemHash* wHash, QObject* obj=nullptr);
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
        void insertGroup(const QModelIndex& groupIndex, WaveDataGroup *grp=nullptr);
        int waveIndex(const QModelIndex& index) const;
        int groupId(const QModelIndex& grpIndex) const;
        WaveData* item(const QModelIndex& index) const;
        WaveItemIndex hashIndex(const QModelIndex& index) const;

        void setGroupPosition(int ypos, const QModelIndex& index);
        void addWaves(const QVector<WaveData*>& wds);

        float cursorTime() const { return mCursorTime; }
        float cursorXpos() const { return mCursorXpos; }

        QSet<int> waveDataIndexSet() const;

        static const char* sStateColor[3];
    };
}
