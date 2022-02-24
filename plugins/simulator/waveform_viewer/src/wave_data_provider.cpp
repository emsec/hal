#include "waveform_viewer/wave_data_provider.h"

namespace hal {
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
        mBuffer = mInputFile.get_buffered_data();
        while (mIndex < mBuffer->mCount && t < mBuffer->mTimeArray[mIndex]) ++mIndex;

        int retval = SaleaeDataTuple::sReadError; // assume empty

        if (mIndex < mBuffer->mCount && mBuffer->mTimeArray[mIndex] == t)
        {
            //exact hit
            retval = mBuffer->mValueArray[mIndex];
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

    SaleaeDataTuple WaveDataProviderFile::nextPoint()
    {
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
