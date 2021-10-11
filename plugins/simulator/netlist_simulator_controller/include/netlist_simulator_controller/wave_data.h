#pragma once

#include <QMap>
#include <QString>
#include <QList>
#include <QSet>
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "netlist_simulator_controller/simulation_input.h"

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
        u32     id()      const { return mId; }
        QString name()    const { return mName; }
        NetType netType() const { return mNetType; }
        int intValue(float t) const;
        void setStartvalue(int val);
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
    };

    class WaveDataList : public QList<WaveData*>
    {
        QMap<u32,int>     mIds;
        u64               mMaxTime;
        void restoreIndex();
        void updateMaxTime();
    public:
        WaveDataList() : mMaxTime(0) {;}
        ~WaveDataList();
        void add(WaveData* wd);
        void addOrReplace(WaveData* wd);
        WaveData* waveDataByNetId(u32 id) const;
        QSet<u32> toSet() const { return mIds.keys().toSet(); }
        void remove(u32 id);
        void incrementMaxTime(u64 deltaT);
        void clearAll();
        void updateClocks();
        u64 maxTime() const { return mMaxTime; }
        void setValueForEmpty(int val);
        void dump() const;
    };
}
