#pragma once

#include "netlist_simulator_controller/wave_data.h"
#include <QHash>
#include <QObject>

namespace hal {

    class WaveIndex : public QObject
    {
        Q_OBJECT
        const WaveDataList*  mWaveDataList;
        QList<int>     mIndexToWave;
        QHash<int,int> mWaveToIndex;

        void updateWaveToIndex();
    public:
        WaveIndex(const WaveDataList* wdl, QObject* parent=nullptr);
        void removeIndex(int inx);
        const WaveDataList* waveDataList() const { return mWaveDataList; }
        void move(int inxFrom, int inxTo);
        WaveData* waveData(int inx) const;
        int numberWavesShown() const { return mIndexToWave.size(); }

    Q_SIGNALS:
        void waveAdded(WaveData* wd);
        void waveDataChanged(int inx);
        void waveRemoved(int inx);

    public Q_SLOTS:
        void addWave();
        void handleWaveReplaced(int iwave);
        void handleWaveUpdated(int iwave);
    };
}
