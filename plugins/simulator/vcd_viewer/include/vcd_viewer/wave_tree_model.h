#pragma once
#include "hal_core/defines.h"
#include <QAbstractItemModel>
#include <QList>

namespace hal {
    class WaveData;
    class WaveDataList;
    class VolatileWaveData;

    class WaveTreeItem
    {
        u32 mId;
        QString mName;
        int mWaveIndex;
    public:
        WaveTreeItem(u32 id_, const QString& nam, int iwave);
        virtual ~WaveTreeItem() {;}
        virtual int childCount() const { return 0; }
        virtual u32 id() const { return mId; }
        virtual QString name() const { return mName; }
        virtual int waveIndex() const { return mWaveIndex; }
        virtual void setName(const QString& nam) { mName = nam; }
    };

    class WaveTreeGroup : public WaveTreeItem, public QList<WaveTreeItem*>
    {
        static u32 sMaxGroupId;

        u32 mGroupId;
    public:
        WaveTreeGroup(const QString& nam = QString());
        u32 groupId() const { return mGroupId; }
    };

    class WaveTreeModel : public QAbstractItemModel
    {
        Q_OBJECT
        WaveDataList* mWaveDataList;
        VolatileWaveData* mVolatileWaveData;
        WaveTreeGroup* mRoot;
        QModelIndex mDragIndex;
        int mValueBase;
        float mCursorPosition;

        WaveTreeItem* item(const QModelIndex& index) const;
        void dropRow(const QModelIndex& parentTo, int row);
        void invalidateParent(const QModelIndex& parentRow);
        void updateVolatile(WaveTreeGroup* grp);
        int  itemValue(const WaveTreeItem* wti) const;
    Q_SIGNALS:
        void inserted(QModelIndex index);
//        void indexInserted(int iwave, bool isVolatile);
        void indexRemoved(int iwave, bool isVolatile);
        void dropped();

    public Q_SLOTS:
        void handleSetValueFormat();
        void handleWaveAdded(int iwave);
        void handleCursorMoved(float xpos);

    public:
        WaveTreeModel(WaveDataList* wdlist, VolatileWaveData* wdVol, QObject* obj=nullptr);
        bool isLeaveItem(const QModelIndex &index) const;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
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

        bool insertItem(int row, const QModelIndex &parent, WaveTreeItem* itm);
        WaveTreeItem* removeItem(int row, const QModelIndex &parent);
        void removeGroup(const QModelIndex& groupIndex);
        void insertGroup(const QModelIndex& groupIndex);
        int waveIndex(const QModelIndex& index) const;
        int groupId(const QModelIndex& grpIndex) const;
        void setVolatilePosition(int ypos, const QModelIndex& index);

        QSet<int> waveDataIndexSet() const;
        QList<QModelIndex> indexByNetIds(const QSet<u32>& netIds);

        static const char* sStateColor[3];
    };
}
