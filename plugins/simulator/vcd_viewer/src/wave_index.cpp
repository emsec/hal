#include "vcd_viewer/wave_index.h"
#include "netlist_simulator_controller/wave_data.h"

namespace hal {
    WaveIndex::WaveIndex(const WaveDataList *wdl, QObject* parent)
        : QObject(parent), mWaveDataList(wdl)
    {
        int n = wdl->size();
        for (int i=0; i<n; i++)
            mIndexToWave.append(i);
        updateWaveToIndex();
        connect(mWaveDataList,&WaveDataList::waveAdded,this,&WaveIndex::addWave);
        connect(mWaveDataList,&WaveDataList::waveReplaced,this,&WaveIndex::handleWaveReplaced);
        connect(mWaveDataList,&WaveDataList::waveUpdated,this,&WaveIndex::handleWaveUpdated);
    }

    void WaveIndex::handleWaveReplaced(int iwave)
    {
        auto it = mWaveToIndex.find(iwave);
        if (it == mWaveToIndex.constEnd()) return;
        Q_EMIT waveDataChanged(it.value());
    }

    void WaveIndex::handleWaveUpdated(int iwave)
    {
        auto it = mWaveToIndex.find(iwave);
        if (it == mWaveToIndex.constEnd()) return;
        Q_EMIT waveDataChanged(it.value());
    }

    WaveData* WaveIndex::waveData(int inx) const
    {
        Q_ASSERT(inx < mIndexToWave.size());
        int iwave = mIndexToWave.at(inx);
        Q_ASSERT(iwave < mWaveDataList->size());
        return mWaveDataList->at(iwave);
    }

    void WaveIndex::removeIndex(int inx)
    {
        Q_ASSERT(inx < mIndexToWave.size());
        mIndexToWave.removeAt(inx);
        updateWaveToIndex();
        Q_EMIT waveRemoved(inx);
    }

    void WaveIndex::updateWaveToIndex()
    {
        mWaveToIndex.clear();
        int inx = 0;
        for (int iwave : mIndexToWave)
        {
            mWaveToIndex[iwave] = inx;
            ++inx;
        }
    }

    void WaveIndex::move(int inxFrom, int inxTo)
    {
        int n = mIndexToWave.size();
        Q_ASSERT(inxFrom < n);
        if (inxTo > n) inxTo = n;

        if (inxFrom == inxTo) return;

        int iwave = mIndexToWave.at(inxFrom);
        if (inxFrom < inxTo)
        {
            mIndexToWave.insert(inxTo,iwave);
            mIndexToWave.removeAt(inxFrom);
        }
        else
        {
            mIndexToWave.removeAt(inxFrom);
            mIndexToWave.insert(inxTo,iwave);
        }
        updateWaveToIndex();
        Q_EMIT(waveDataChanged(-1));
    }

    void WaveIndex::addWave()
    {
        Q_ASSERT(!mWaveDataList->isEmpty());
        int inx = mIndexToWave.size();
        int iwave = mWaveDataList->size()-1;
        mWaveToIndex[iwave] = inx;
        mIndexToWave.append(iwave);
        Q_EMIT waveAdded(mWaveDataList->at(iwave));
    }

}
