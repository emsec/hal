#include "waveform_viewer/wave_data_provider.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"

namespace hal {
    void WaveDataProvider::setGroup(bool grp, int bts, int base)
    {
        mGroup = grp;
        mBits = bts;
        mValueBase = base;
    }

    int WaveDataProviderMap::startValue(u64 t)
    {
        mIter = mDataMap.lowerBound(t);

        int retval = SaleaeDataTuple::sReadError; // assume empty

        if (mIter != mDataMap.constEnd() && mIter.key() == t)
        {
            //exact hit
            retval = mIter.value();
            ++mIter;
        }
        else if (mIter != mDataMap.constBegin())
        {
            //value of last data point before entering t-range
            --mIter;
            retval = mIter.value();
            ++mIter;
        }
        else if (mIter != mDataMap.constEnd())
        {
            //no previous data but not empty
            retval = -1;
        }
        return retval;
    }

    SaleaeDataTuple WaveDataProviderMap::nextPoint()
    {
        if (mIter==mDataMap.constEnd())
            return SaleaeDataTuple();

        SaleaeDataTuple retval(mIter.key(),mIter.value());
        ++mIter;
        return retval;
    }

    WaveDataProviderFile::~WaveDataProviderFile()
    {
        if (mBuffer) delete mBuffer;
    }

    int WaveDataProviderFile::startValue(u64 t)
    {
        mIndex = 0;
        mDataMap.clear();
        mStoreData = mTimeframe.hasUserTimeframe() ? Recording : Off;

        mBuffer = mInputFile.get_buffered_data();
        while (mIndex < mBuffer->mCount && t < mBuffer->mTimeArray[mIndex])
        {
            if (isRecording()) storeCurrentDatapoint();
            ++mIndex;
        }

        int retval = SaleaeDataTuple::sReadError; // assume empty

        if (mIndex < mBuffer->mCount && mBuffer->mTimeArray[mIndex] == t)
        {
            //exact hit
            retval = mBuffer->mValueArray[mIndex];
            if (isRecording()) storeCurrentDatapoint();
            ++mIndex;
        }
        else if (mIndex)
        {
            //value of last data point before entering t-range
            --mIndex;
            retval = mBuffer->mValueArray[mIndex];
            ++mIndex;
        }
        else if (mIndex < mBuffer->mCount)
        {
            //no previous data but not empty
            retval = -1;
        }
        return retval;
    }

    void WaveDataProviderFile::storeCurrentDatapoint()
    {
        if (mBuffer->mTimeArray[mIndex] < mTimeframe.sceneMinTime())
            mDataMap[mTimeframe.sceneMinTime()] = mBuffer->mValueArray[mIndex];
        else if (mBuffer->mTimeArray[mIndex] > mTimeframe.sceneMaxTime())
            mStoreData = Complete;
        else
        {
            mDataMap[mBuffer->mTimeArray[mIndex]] = mBuffer->mValueArray[mIndex];
            if (mDataMap.size() > NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable())
            {
                mStoreData = Failed;
                mDataMap.clear();
            }
            else if (mIndex >= mBuffer->mCount-1 || mBuffer->mTimeArray[mIndex] == mTimeframe.sceneMaxTime())
                mStoreData = Complete;
        }
    }

    SaleaeDataTuple WaveDataProviderFile::nextPoint()
    {
        if (isRecording()) storeCurrentDatapoint();

        if (mIndex >= mBuffer->mCount)
            return SaleaeDataTuple();

        SaleaeDataTuple retval(mBuffer->mTimeArray[mIndex],mBuffer->mValueArray[mIndex]);
        ++mIndex;
        return retval;
    }

    int WaveDataProviderClock::valueForTransition() const
    {
        return mClock.start_at_zero ? (mTransition%2) : 1 - (mTransition%2);
    }

    int WaveDataProviderClock::startValue(u64 t)
    {
        mTransition = t / mClock.switch_time;
        int retval = valueForTransition();
        ++mTransition;
        return retval;
    }

    SaleaeDataTuple WaveDataProviderClock::nextPoint()
    {
        SaleaeDataTuple retval(mTransition * mClock.switch_time, valueForTransition());
        ++mTransition;
        return retval;
    }
}
