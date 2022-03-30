#include "waveform_viewer/wave_data_provider.h"
#include "netlist_simulator_controller/wave_data.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/simulation_settings.h"
#include <QDebug>

namespace hal {
    void WaveDataProvider::setGroup(bool grp, int bts, int base)
    {
        mGroup = grp;
        mBits = bts;
        mValueBase = base;
    }

    SaleaeDataTuple WaveDataProviderMap::startValue(u64 t)
    {
        mIter = mDataMap.lowerBound(t);

        SaleaeDataTuple retval; // assume empty

        if (mIter != mDataMap.constEnd() && mIter.key() == t)
        {
            //exact hit
            retval.mTime = mIter.key();
            retval.mValue = mIter.value();
            ++mIter;
        }
        else if (mIter != mDataMap.constBegin())
        {
            //value of last data point before entering t-range
            --mIter;
            retval.mTime = mIter.key();
            retval.mValue = mIter.value();
            ++mIter;
        }
        else if (mIter != mDataMap.constEnd())
        {
            //no previous data but not empty
            retval.mTime = t;
            retval.mValue = -1;
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

    SaleaeDataTuple WaveDataProviderFile::startValue(u64 t)
    {
        mDataMap.clear();
        mStoreData = mTimeframe.hasUserTimeframe() ? Recording : Off;

        SaleaeDataTuple lastval;
        SaleaeDataTuple retval;

        bool skipData = true;

        while (skipData && mInputFile.good())
        {
            if (mBuffer) delete mBuffer;
            mBuffer = mInputFile.get_buffered_data(NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable());
            mIndex = 0;
            if (!mBuffer) return SaleaeDataTuple();

            while (mIndex < mBuffer->mCount && mBuffer->mTimeArray[mIndex] < t)
            {
                if (isRecording()) storeCurrentDatapoint();
                lastval.mTime = mBuffer->mTimeArray[mIndex];
                lastval.mValue = mBuffer->mValueArray[mIndex];
                ++mIndex;
            }

            skipData = (mBuffer->mTimeArray[mIndex] < t);
        }

        if (mIndex < mBuffer->mCount && mBuffer->mTimeArray[mIndex] == t)
        {
            //exact hit
            retval.mTime = mBuffer->mTimeArray[mIndex];
            retval.mValue = mBuffer->mValueArray[mIndex];
            if (isRecording()) storeCurrentDatapoint();
            ++mIndex;
        }
        else if (mIndex)
        {
            //value of last data point before entering t-range
            retval = lastval;
        }
        else if (mIndex < mBuffer->mCount)
        {
            //no previous data but not empty
            retval.mTime = t;
            retval.mValue = -1;
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
        {
            if (!mInputFile.good()) return SaleaeDataTuple();
            if (mBuffer) delete mBuffer;
            mBuffer = mInputFile.get_buffered_data(NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable());
            mIndex = 0;
            if (!mBuffer) return SaleaeDataTuple();
       }

        SaleaeDataTuple retval(mBuffer->mTimeArray[mIndex],mBuffer->mValueArray[mIndex]);
        ++mIndex;
        return retval;
    }

    int WaveDataProviderClock::valueForTransition() const
    {
        return mClock.start_at_zero ? (mTransition%2) : 1 - (mTransition%2);
    }

    SaleaeDataTuple WaveDataProviderClock::startValue(u64 t)
    {
        mTransition = t / mClock.switch_time;
        SaleaeDataTuple retval(mTransition * mClock.switch_time, valueForTransition());
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
