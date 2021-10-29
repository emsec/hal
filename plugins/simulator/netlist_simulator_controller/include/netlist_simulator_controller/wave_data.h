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

    class NetlistSimulator;
    class Net;

    class WaveData : public QMap<u64,int>
    {
    public:
        enum NetType { RegularNet, InputNet, OutputNet, ClockNet };
    private:
        u32 mId;
        QString mName;
        NetType mNetType;

        QMap<u64,int>::const_iterator timeIterator(float t) const;
    public:
        WaveData(u32 id_, const QString& nam, NetType tp = RegularNet,
                 const QMap<u64,int>& other = QMap<u64,int>() );
        WaveData(const Net* n, NetType tp = RegularNet);
        virtual ~WaveData();
        u32     id()      const { return mId; }
        QString name()    const { return mName; }
        NetType netType() const { return mNetType; }
        int intValue(float t) const;
        u64  maxTime() const;
        void setStartvalue(int val);
        void setId(u32 id_) { mId = id_; }
        void insertBooleanValue(u64 t, BooleanFunction::Value bval);
        bool insertToggleTime(u64 t);
        char charValue(float t) const;
        char charValue(const QMap<u64,int>::const_iterator& it) const;
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

    class WaveDataList : public QObject, public QList<WaveData*>
    {
        Q_OBJECT

        QMap<u32,int>     mIds;
        u64               mMaxTime;
        void restoreIndex();
        void updateMaxTime();
        void setMaxTime(u64 tmax);
    public:
        WaveDataList(QObject* parent = nullptr) : QObject(parent), mMaxTime(0) {;}
        ~WaveDataList();
        void add(WaveData* wd);
        void addOrReplace(WaveData* wd);
        void replaceWave(int inx, WaveData *wd);
        WaveData* waveDataByNetId(u32 id) const;
        QSet<u32> toSet() const { return mIds.keys().toSet(); }
        void remove(u32 id);
        void incrementMaxTime(u64 deltaT);
        void clearAll();
        void updateClocks();
        u64 maxTime() const { return mMaxTime; }
        void setValueForEmpty(int val);
        void dump() const;
        QList<const WaveData*> toList() const;
        QList<const WaveData*> partialList(u64 start_time, u64 end_time, std::set<const Net*>& nets) const;
    Q_SIGNALS:
        void waveAdded(int inx);
        void waveDataAboutToBeChanged(int inx);
        void waveReplaced(int inx);
        void waveUpdated(int inx);
        void waveRemoved(int inx);
        void maxTimeChanged(u64 tmax);
    };
}
