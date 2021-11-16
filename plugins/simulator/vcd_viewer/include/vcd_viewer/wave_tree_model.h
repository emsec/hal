#pragma once
#include "hal_core/defines.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QAbstractItemModel>
#include <QList>

namespace hal {
    class WaveData;
    class WaveDataList;
    class WaveDataGroup;

    class WaveDataRoot : public WaveDataGroup
    {
    public:
        WaveDataRoot(WaveDataList* wdList)
            : WaveDataGroup(wdList,"root") {;}
        void recalcData() override {;}
    };

    class WaveTreeModel : public QAbstractItemModel
    {
        Q_OBJECT
        WaveDataList* mWaveDataList;
        WaveDataRoot* mRoot;
        QModelIndex mDragIndex;
        float mCursorPosition;
        bool mIgnoreSignals;

        WaveData* item(const QModelIndex& index) const;
        void dropRow(const QModelIndex& parentTo, int row);
        void invalidateParent(const QModelIndex& parentRow);
        void updateGroup(WaveDataGroup* grp);
    Q_SIGNALS:
        void inserted(QModelIndex index);
//        void indexInserted(int iwave, bool isGroup);
        void indexRemoved(int iwave, bool isGroup);
        void triggerReorder();

    public Q_SLOTS:
        void handleUpdateValueFormat();
        void handleWaveAdded(int iwave);
        void handleWaveMovedToGroup(int iwave, WaveDataGroup* grp);
        void handleGroupAdded(int grpId);
        void handleGroupAboutToBeRemoved(WaveDataGroup* grp);
        void handleCursorMoved(float xpos);

    public:
        WaveTreeModel(WaveDataList* wdlist, QObject* obj=nullptr);
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
        void setDragIndex(const QModelIndex& index) { mDragIndex = index; }

        bool insertItem(int row, const QModelIndex &parent, WaveData* wd);
        WaveData* removeItem(int row, const QModelIndex &parent);
        void removeGroup(const QModelIndex& groupIndex);
        void insertGroup(const QModelIndex& groupIndex, WaveDataGroup *grp=nullptr);
        int waveIndex(const QModelIndex& index) const;
        int groupId(const QModelIndex& grpIndex) const;
        void setGroupPosition(int ypos, const QModelIndex& index);

        QSet<int> waveDataIndexSet() const;

        static const char* sStateColor[3];
    };
}
