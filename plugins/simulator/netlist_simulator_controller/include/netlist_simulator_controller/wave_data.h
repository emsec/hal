#pragma once

#include <QMap>
#include <QString>
#include <QList>
#include <QSet>
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"

namespace hal {

    class NetlistSimulator;
    class Net;

    class WaveData : public QMap<u64,int>
    {
        u32 mId;
        QString mName;
    public:
        WaveData(u32 id_, const QString& nam, const QMap<u64,int>& other = QMap<u64,int>() );
        WaveData(const Net* n);
        u32 id() const { return mId; }
        QString name() const { return mName; }
        int tValue(float t) const;
        void setStartvalue(int val);
        void insertBooleanValue(u64 t, BooleanFunction::Value bval);
        bool insertToggleTime(u64 t);
        QString textValue(const QMap<u64,int>::const_iterator& it) const;
        static WaveData* clockFactory(const Net* n, int start, int period, int duration);
//        static WaveData* simulationResultFactory(Net* n, const NetlistSimulator* sim);
    };

    class WaveDataList : public QList<WaveData*>
    {
        QMap<u32,int>     mIds;
        void restoreIndex();
    public:
        ~WaveDataList();
        void add(WaveData* wd);
        void addOrReplace(WaveData* wd);
        WaveData* waveDataByNetId(u32 id) const;
        QSet<u32> toSet() const { return mIds.keys().toSet(); }
        void remove(u32 id);
    };
}
