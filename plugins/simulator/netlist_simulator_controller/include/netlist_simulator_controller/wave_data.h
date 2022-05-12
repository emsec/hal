#pragma once

#include <QMap>
#include <QString>
#include <QList>
#include <QSet>
#include <QObject>
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/saleae_directory.h"
#include <set>

namespace hal {

    class WaveData;
    class NetlistSimulator;
    class SaleaeInputFile;
    class Net;

    class WaveDataTimeframe
    {
        friend class WaveDataList;
        u64 mSceneMaxTime;
        u64 mSimulateMaxTime;
        u64 mUserdefMaxTime;
        u64 mUserdefMinTime;
        static const int sMinSceneWidth = 1000;
    public:
        WaveDataTimeframe();
        u64 sceneMaxTime() const;
        u64 simulateMaxTime() const;
        u64 sceneMinTime() const;
        u64 sceneWidth() const;
        bool hasUserTimeframe() const;
        void setUserTimeframe(u64 t0=0, u64 t1=0);
        void setSceneMaxTime(u64 t);
        void setSimulateMaxTime(u64 t) { mSimulateMaxTime = t; }
    };

    class WaveDataList;
    class WaveDataTrigger;

    class WaveData
    {
    public:
        enum NetType { RegularNet, InputNet, OutputNet, ClockNet, BooleanNet, TriggerTime, NetGroup };
        enum LoadPolicy { TooBigToLoad, LoadTimeframe, LoadAllData };
    private:
        u32 mId;
        int mFileIndex;
        u64 mFileSize;
        u64 mTimeframeSize;
        QString mName;
        NetType mNetType;
        int mBits;
    protected:
        int mValueBase;
        QMap<u64,int> mData;
        bool mDirty;
        WaveDataList* mWaveDataList;

        QMap<u64,int>::const_iterator timeIterator(double t) const;
        void resetWave();
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
        bool    isDirty()                   const { return mDirty; }
        const QMap<u64,int>& data()         const { return mData; }
        int     fileIndex()                 const { return mFileIndex; }
        u64     fileSize()                  const { return mFileSize; }
        int     valueBase()                 const { return mValueBase; }
        std::string fileName()              const;
        SaleaeDirectoryNetEntry::Type composedType() const;
        void setId(u32 id_);
        bool rename(const QString& nam);
        void setBits(int bts);
        void setDirty(bool dty)                     { mDirty = dty; }
        void setFileIndex(int saleaIndex)           { mFileIndex = saleaIndex; }
        void setFileSize(u64 siz);
        void setTimeframeSize(u64 siz)              { mTimeframeSize = siz; }
        void setWaveDataList(WaveDataList* wdList)  { mWaveDataList = wdList; }
        virtual LoadPolicy loadPolicy() const;
        int dataIndex() const;

        virtual u64 neighborTransition(double t, bool next) const;
        void loadDataUnlessAlreadyLoaded();
        bool loadSaleae(const WaveDataTimeframe& tframe = WaveDataTimeframe());
        void saveSaleae();
        void setData(const QMap<u64,int>& dat);
        virtual int  intValue(double t) const;
        int get_value_at(u64 t) const;
        std::string get_name() const { return mName.toStdString(); }
        std::vector<std::pair<u64,int>> get_events(u64 t0 = 0) const;
        std::vector<std::pair<u64,int>> get_triggered_events(const WaveDataTrigger* wdTrig, u64 t0 = 0);
        u64  maxTime() const;
        void clear() { mData.clear(); }
        void insert(u64 t, int val) { mData.insert(t,val); }
        void insertBooleanValueWithoutSync(u64 t, BooleanFunction::Value bval);
        QString strValue(int val) const;
        QString strValue(double t) const;
        QString strValue(const QMap<u64,int>::const_iterator& it) const;
        void setValueBase(int bas) { mValueBase = bas; }
        bool isEqual(const WaveData& other, int tolerance=0) const;
        static QString stringValue(int val, int bits, int base);
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
    class WaveDataBoolean;

    class WaveDataList : public QObject, public QList<WaveData*>
    {
        friend class WaveDataGroup;
        friend class WaveDataBoolean;
        friend class WaveDataTrigger;
        Q_OBJECT

        QMap<u32,int>     mIds;
        WaveDataTimeframe mTimeframe;
        SaleaeDirectory   mSaleaeDirectory;
        u32               mMaxGroupId;
        u32               mMaxBooleanId;
        u32               mMaxTriggerid;
        void testDoubleCount();
        void restoreIndex();
        void updateMaxTime();
        void setMaxTime(u64 tmax);

        using QList<WaveData*>::append;
        using QList<WaveData*>::insert;

        void replaceWaveData(int inx, WaveData *wdNew);
        void registerGroup(WaveDataGroup* grp);
        void registerBoolean(WaveDataBoolean* wdBool);
        void registerTrigger(WaveDataTrigger* wdTrig);
    public:
        /**
         * Map of groups indexed by non-zero group id
         */

        QMap<u32,WaveDataGroup*> mDataGroups;
        QMap<u32,WaveDataBoolean*> mDataBooleans;
        QMap<u32,WaveDataTrigger*> mDataTrigger;
        WaveDataList(const QString& sdFilename, QObject* parent = nullptr);
        ~WaveDataList();

        u32  nextGroupId() { return ++mMaxGroupId; }
        u32  maxGroupId() const { return mMaxGroupId; }
        u32  nextBooleanId() { return ++ mMaxBooleanId; }
        u32  nextTriggerId() { return ++ mMaxTriggerid; }
        void addWavesToGroup(u32 grpId, const QVector<WaveData*>& wds);
        void removeGroup(u32 grpId);

        void addOrReplace(WaveData* wd);
        void add(WaveData* wd, bool updateSaleae);
        void remove(u32 id);
        void incrementSimulTime(u64 deltaT);
        void clearAll();
        void updateClocks();
        void updateWaveData(int inx);

        WaveData* waveDataByNet(const Net* n);
        WaveData* waveDataByName(const std::string& nam) const;
        int waveIndexByNetId(u32 id) const { return mIds.value(id,-1); }
        void triggerAddToView(u32 id) const;
        bool hasNet(u32 id) const { return mIds.contains(id); }
        QSet<u32> toSet() const;
        void updateWaveName(int iwave, const QString& nam);
        void updateGroupName(u32 grpId, const QString& nam);
        const WaveDataTimeframe& timeFrame() const { return mTimeframe; }
        void setValueForEmpty(int val);
        void dump() const;
        QList<const WaveData*> partialList(u64 start_time, u64 end_time, const std::set<const Net*>& nets) const;
        void emitWaveAdded(int inx);
        void emitWaveUpdated(int inx);
        void emitGroupUpdated(int grpId);
        void emitWaveRemovedFromGroup(int iwave, int grpId);
        void emitTimeframeChanged();
        void updateFromSaleae();
        SaleaeDirectory& saleaeDirectory() { return mSaleaeDirectory; }
        void insertBooleanValue(WaveData* wd, u64 t, BooleanFunction::Value bval);
        void setUserTimeframe(u64 t0=0, u64 t1=0);
    Q_SIGNALS:
        void waveAdded(int inx) const;
        void groupAdded(int grpId);
        void booleanAdded(int boolId);
        void triggerAdded(int trigId);
        void groupAboutToBeRemoved(WaveDataGroup* grp);
        void waveDataAboutToBeChanged(int inx);
        void waveUpdated(int inx, int grpId);
        void groupUpdated(int grpId);
        void waveRenamed(int iwave);
        void groupRenamed(int grpId);
        void waveRemoved(int inx);
        void waveAddedToGroup(const QVector<u32>& netIds, int grpId);
        void waveRemovedFromGroup(int iwave, int grpId);
        void timeframeChanged(const WaveDataTimeframe* tframe);
        void triggerBeginResetModel();
        void triggerEndResetModel();
    };

    class WaveDataGroupIndex {
        friend uint qHash(const WaveDataGroupIndex& wdgi);
        uint mCode;
        void construct(u32 id, bool isNet);
    public:
        WaveDataGroupIndex(const WaveData* wd);
        WaveDataGroupIndex(u32 id, bool isNet) { construct(id, isNet); }
        bool operator==(const WaveDataGroupIndex& other) const { return mCode == other.mCode; }
    };

    uint qHash(const WaveDataGroupIndex& wdgi);

    class WaveDataBoolean : public WaveData
    {
        int mInputCount;
        WaveData** mInputWaves;
        QHash<WaveDataGroupIndex,int> mIndex;
        char* mTruthTable;
    public:
        WaveDataBoolean(WaveDataList* wdList, QString boolFunc);
        WaveDataBoolean(WaveDataList* wdList, const QList<WaveData*>& boolInput, const QList<int>& acceptMask);
        ~WaveDataBoolean();
        void recalcData();
        virtual LoadPolicy loadPolicy() const override;
        QList<WaveData*> children() const;
        const char* truthTable() const { return mTruthTable; }
        virtual int intValue(double t) const override;
    };

    class WaveDataTrigger : public WaveData
    {
        int mTriggerCount;
        WaveData** mTriggerWaves;
        WaveData* mFilterWave;
        QHash<WaveDataGroupIndex,int> mIndex;
        int* mToValue;
    public:
        WaveDataTrigger(WaveDataList* wdList, const QList<WaveData*>& wdTrigger, const QList<int>& toVal = QList<int>());
        ~WaveDataTrigger();
        void recalcData();
        virtual LoadPolicy loadPolicy() const override;
        QList<WaveData*> children() const;
        virtual u64 neighborTransition(double t, bool next) const override;
        virtual int intValue(double t) const override;
        void set_filter_wave(WaveData* wd);
        QList<int> toValueList() const;
        WaveData* get_filter_wave() const { return mFilterWave; }
    };

    class WaveDataGroup : public WaveData
    {

    protected:
        QList<WaveData*> mGroupList;

        QHash<WaveDataGroupIndex,int> mIndex;
    public:
        WaveDataGroup(WaveDataList* wdList, int grpId, const QString& nam);
        WaveDataGroup(WaveDataList* wdList, const QString& nam = QString());
        WaveDataGroup(WaveDataList* wdList, const WaveData* wdGrp);
        virtual ~WaveDataGroup();
        virtual int bits() const override;
        virtual int size() const { return mGroupList.size(); }
        void addNet(const Net* n);
        virtual void insert(int inx, WaveData* wd);
        virtual void addWaves(const QVector<WaveData*>& wds);
        void restoreIndex();
        virtual void recalcData();
        virtual bool hasNetId(u32 id) const;
        virtual QList<WaveData*> children() const;
        QList<int> childrenWaveIndex() const;
        virtual WaveData* childAt(int inx) const;
        virtual WaveData* removeAt(int inx);
        virtual bool isEmpty() const { return mGroupList.isEmpty(); }
        virtual void updateWaveData(WaveData* wd);
        virtual int childIndex(WaveData* wd) const;
        virtual int netIndex(u32 id) const;
        virtual void replaceChild(WaveData* wd);
        virtual LoadPolicy loadPolicy() const override;
        void add_waveform(WaveData* wd);
        void remove_waveform(WaveData* wd);
        std::vector<WaveData*> get_waveforms() const;
        virtual int intValue(double t) const override;
    };
}
