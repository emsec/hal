#pragma once

#include <QMap>
#include <QString>
#include <QList>
#include <QSet>
#include <QObject>
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "netlist_simulator_controller/simulation_input.h"
#include <set>

namespace hal {

    class WaveData;
    class NetlistSimulator;
    class Net;

    class WaveGraphicsItem
    {
    public:
        WaveGraphicsItem() {;}
        virtual ~WaveGraphicsItem() {;}
        virtual void updateGraphicsItem(WaveData* wd) = 0;
        virtual void removeGraphicsItem()     = 0;
        virtual void setItemVisible(bool vis) = 0;
    };

    class WaveData
    {
    public:
        enum NetType { RegularNet, InputNet, OutputNet, ClockNet, NetGroup };
    private:
        u32 mId;
        QString mName;
        NetType mNetType;
        int mBits;
        int mValueBase;
    protected:
        QMap<u64,int> mData;
        WaveGraphicsItem* mGraphicsItem;

        QMap<u64,int>::const_iterator timeIterator(float t) const;
    public:
        WaveData(const WaveData& other);
        WaveData(u32 id_, const QString& nam, NetType tp = RegularNet,
                 const QMap<u64,int>& dat = QMap<u64,int>() );
        WaveData(const Net* n, NetType tp = RegularNet);
        virtual ~WaveData();
        u32     id()                        const { return mId; }
        QString name()                      const { return mName; }
        NetType netType()                   const { return mNetType; }
        virtual int bits()                  const { return mBits; }
        WaveGraphicsItem*    graphicsItem() const { return mGraphicsItem; }
        const QMap<u64,int>& data()         const { return mData; }
        void setId(u32 id_)                         { mId = id_; }
        void setName(const QString& nam)            { mName = nam; }
        void setBits(int bts)                       { mBits = bts; }
        void setGraphicsItem(WaveGraphicsItem* wgi) { mGraphicsItem = wgi; }
        void setData(const QMap<u64,int>& dat);
        int  intValue(float t) const;
        u64  maxTime() const;
        void clear() { mData.clear(); }
        void insert(u64 t, int val) { mData.insert(t,val); }
        void setStartvalue(int val);
        void insertBooleanValue(u64 t, BooleanFunction::Value bval);
        void eraseAtTime(u64 t);
        bool insertToggleTime(u64 t);
        QString strValue(float t) const;
        QString strValue(const QMap<u64,int>::const_iterator& it) const;
        void setValueBase(int bas) { mValueBase = bas; }
//        static WaveData* simulationResultFactory(Net* n, const NetlistSimulator* sim);
    };

    class WaveDataClock : public WaveData
    {
        SimulationInput::Clock mClock;
        u64 mMaxTime;
        void dataFactory();
    public:
        WaveDataClock(const Net* n, const SimulationInput::Clock& clk, u64 tmax);
        WaveDataClock(const Net* n, int start, u64 period, u64 tmax);
        void setMaxTime(u64 tmax);
        SimulationInput::Clock clock() const { return mClock; }
    };

    class WaveDataGroup;

    class WaveDataList : public QObject, public QList<WaveData*>
    {
        Q_OBJECT

        QMap<u32,int>     mIds;
        u64               mMaxTime;
        void restoreIndex();
        void updateMaxTime();
        void setMaxTime(u64 tmax);
    public:
        QMap<u32,WaveDataGroup*> mDataGroups;
        WaveDataList(QObject* parent = nullptr) : QObject(parent), mMaxTime(0) {;}
        ~WaveDataList();
        void add(WaveData* wd, bool silent);
        void addGroup(WaveDataGroup* grp);
        u32  createGroup(QString grpName, const QVector<u32>& netIds);
        void removeGroup(u32 grpId);
        void addOrReplace(WaveData* wd);
        void replaceWaveData(int inx, const WaveData *wdNew);
        WaveData* waveDataByNetId(u32 id) const;
        int waveIndexByNetId(u32 id) const { return mIds.value(id,-1); }
        bool hasNet(u32 id) const { return mIds.contains(id); }
        QSet<u32> toSet() const;
        void remove(u32 id);
        void incrementMaxTime(u64 deltaT);
        void clearAll();
        void updateClocks();
        void updateWaveData(int inx);
        u64 maxTime() const { return mMaxTime; }
        void setValueForEmpty(int val);
        void dump() const;
        QList<const WaveData*> toList() const;
        QList<const WaveData*> partialList(u64 start_time, u64 end_time, std::set<const Net*>& nets) const;
    Q_SIGNALS:
        void waveAdded(int inx);
        void groupAdded(int grpId);
        void groupAboutToBeRemoved(WaveDataGroup* grp);
        void waveDataAboutToBeChanged(int inx);
        void waveUpdated(int inx);
        void waveRemoved(int inx);
        void waveMovedToGroup(int inx, WaveDataGroup* grp);
        void maxTimeChanged(u64 tmax);
    };

    class WaveDataGroup : public WaveData
    {
        static u32 sMaxGroupId;

        WaveDataList* mWaveDataList;
        QList<WaveData*> mGroupList;
        QMap<u32,int>    mIds;
        void restoreIndex();
    public:
        WaveDataGroup(WaveDataList* wdList, const QString& nam = QString());
        WaveDataGroup(WaveDataList* wdList, const WaveData* wdGrp);
        virtual ~WaveDataGroup();
        virtual int bits() const;
        virtual int size() const { return mGroupList.size(); }
        void addNet(const Net* n);
        virtual void insert(int inx, WaveData* wd);
        virtual void recalcData();
        virtual bool hasNetId(u32 id) const { return mIds.contains(id); }
        virtual QList<WaveData*> children() const;
        virtual WaveData* childAt(int inx) const;
        virtual WaveData* removeAt(int inx);
        virtual bool isEmpty() const { return mGroupList.isEmpty(); }
        virtual void updateWaveData(WaveData* wd);
        virtual int childIndex(WaveData* wd) const;
    };
}
