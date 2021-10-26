#include "vcd_viewer/wave_index.h"
#include "netlist_simulator_controller/wave_data.h"

namespace hal {
    WaveIndex::WaveIndex(WaveDataList *wdl, QObject* parent)
        : QObject(parent), mWaveDataList(wdl), mAutoAddWaves(false)
    {
        int n = wdl->size();
        for (int i=0; i<n; i++)
            mIndexToWave.append(i);
        updateWaveToIndex();
        connect(mWaveDataList,&WaveDataList::waveReplaced,this,&WaveIndex::handleWaveReplaced);
        connect(mWaveDataList,&WaveDataList::waveUpdated,this,&WaveIndex::handleWaveUpdated);
        setAutoAddWaves(true);
    }

    void WaveIndex::setAutoAddWaves(bool enable)
    {
        if (enable == mAutoAddWaves) return;
        if ((mAutoAddWaves = enable))
            connect(mWaveDataList,&WaveDataList::waveAdded,this,&WaveIndex::addWave);
        else
            disconnect(mWaveDataList,&WaveDataList::waveAdded,this,&WaveIndex::addWave);
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

    void WaveIndex::setWaveData(int inx, WaveData* wd)
    {
        Q_ASSERT(inx < mIndexToWave.size());
        int iwave = mIndexToWave.at(inx);
        Q_ASSERT(iwave < mWaveDataList->size());
        mWaveDataList->replaceWave(iwave,wd);
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

    QSet<int> WaveIndex::waveDataIndexSet() const
    {
        return mIndexToWave.toSet();
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

    void WaveIndex::addWave(int iwave)
    {
        if (mWaveToIndex.contains(iwave)) return;
        Q_ASSERT(!mWaveDataList->isEmpty());
        int inx = mIndexToWave.size();
        mWaveToIndex[iwave] = inx;
        mIndexToWave.append(iwave);
        Q_EMIT waveAppended(mWaveDataList->at(iwave));
    }

}
