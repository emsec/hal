#pragma once

#include "netlist_simulator_controller/wave_data.h"
#include <QHash>
#include <QObject>
#include <QSet>

namespace hal {

    class WaveIndex : public QObject
    {
        Q_OBJECT
        WaveDataList*  mWaveDataList;
        QList<int>     mIndexToWave;
        QHash<int,int> mWaveToIndex;
        bool mAutoAddWaves;

        void updateWaveToIndex();
    public:
        WaveIndex(WaveDataList* wdl, QObject* parent=nullptr);
        void removeIndex(int inx);
        const WaveDataList* waveDataList() const { return mWaveDataList; }
        void move(int inxFrom, int inxTo);
        WaveData* waveData(int inx) const;
        void setWaveData(int inx, WaveData *wd);
        int numberWavesShown() const { return mIndexToWave.size(); }
        void setAutoAddWaves(bool enable);
        QSet<int> waveDataIndexSet() const;

    Q_SIGNALS:
        void waveAppended(WaveData* wd);
        void waveDataChanged(int inx);
        void waveRemoved(int inx);

    public Q_SLOTS:
        void addWave(int iwave);
        void handleWaveReplaced(int iwave);
        void handleWaveUpdated(int iwave);
    };
}
