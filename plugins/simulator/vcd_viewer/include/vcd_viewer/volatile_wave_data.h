#pragma once

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QHash>
#include "netlist_simulator_controller/wave_data.h"

namespace hal {

    class VolatileWaveData : public QObject
    {
        Q_OBJECT
    public:
        struct VolatileWaveItem
        {
            u32 groupId;
            int yPosition;
            WaveData* wd;
        };

    private:
        QMap<u32,WaveData*> mDataMap;
        QMap<u32,int> mYposition;
//        QMutex* mMutex;

    Q_SIGNALS:
        void triggerRepaint();

    public:
        VolatileWaveData(QObject *parent = nullptr);
        ~VolatileWaveData();
        void addOrReplace(u32 id, WaveData* wd);
        void clear();
        bool hasGroup(u32 id) const { return mDataMap.contains(id); }
        void setYposition(u32 id, int ypos) { mYposition[id] = ypos; }
        WaveData* waveData(u32 id) const { return mDataMap.value(id); }
        void ready();
        QList<VolatileWaveItem> yPositionData() const;
    };
}
