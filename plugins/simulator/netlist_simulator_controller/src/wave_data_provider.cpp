#include "netlist_simulator_controller/wave_data_provider.h"
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

    void WaveDataProvider::setBoolean(bool bl, int bts)
    {
        mBoolean = bl;
        mBits = bts;
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

    //-----------------------------------------------------
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
        if (!mInputFile.good())
            return retval;

        bool skipData = true;

        int loadCycle = 0;

        while (skipData && mInputFile.good())
        {
            if (mBuffer) delete mBuffer;
            mBuffer = mInputFile.get_buffered_data(NetlistSimulatorControllerPlugin::sSimulationSettings->maxSizeLoadable());
            mIndex = 0;
            ++loadCycle;
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
        else if (mIndex || loadCycle > 1)
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

    //-----------------------------------------------------
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

    //-----------------------------------------------------
    WaveDataProviderGroup::WaveDataProviderGroup(const std::string& saleaeDirectoryPath, const QList<WaveData*>& wdList)
        : mParser(saleaeDirectoryPath), mBitMask(nullptr), mCurrentTime(0), mSampleValue(-1), mValuePending(false), mEventReady(false)
    {
        if (wdList.isEmpty()) return;
        int n = wdList.size();
        mBitMask = new u32[n];
        mLastValue = WaveGroupValue(n);
        for (int i=0; i<n; i++)
        {
            WaveData* wd = wdList.at(i);
            mBitMask[i] = 1 << i;
            mParser.register_callback(wd->name().toStdString(), wd->id(), [this](const void* obj, uint64_t t, int val) {
                if (t != mCurrentTime)
                {
                    mNextValue.mergePrevious(mLastValue);
                    mSampleValue = mNextValue.value();
                    mSampleTime = mCurrentTime;
                    mLastValue = mNextValue;
                    mCurrentTime = t;
                    mEventReady = true;
                }
                int mask = *((int*) obj);
                mNextValue.setValue(mask,val);
            }, mBitMask+i);
        }
    }

    WaveDataProviderGroup::~WaveDataProviderGroup()
    {
        if (mBitMask) delete [] mBitMask;
    }

    bool WaveDataProviderGroup::nextEventReady()
    {
        while (mParser.next_event())
        {
            if (mEventReady)
            {
                mEventReady = false;
                return true;
            }
        }
        return false;
    }

    SaleaeDataTuple WaveDataProviderGroup::startValue(u64 t)
    {
        u64 lastT = 0;
        int lastVal = -1;
        SaleaeDataTuple retval;
        while (nextEventReady())
        {
            if (mSampleTime < t)
            {
                lastT = mSampleTime;
                lastVal = mSampleValue;
            }
            else
            {
                if (mSampleTime == t)
                {
                    retval.mTime = mSampleTime;
                    retval.mValue = mSampleValue;
                }
                else
                {
                    retval.mTime = t;
                    retval.mValue = lastVal;
                    mValuePending = true;
                }
                return retval;
            }
        }
        return retval;
    }

    SaleaeDataTuple WaveDataProviderGroup::nextPoint()
    {
        SaleaeDataTuple retval;
        if (mValuePending)
        {
            mValuePending = false;
            retval.mTime = mSampleTime;
            retval.mValue = mSampleValue;
            return retval;
        }
        if (nextEventReady())
        {
            retval.mTime = mSampleTime;
            retval.mValue = mSampleValue;
        }
        return retval;
    }

    //-----------------------------------------------------
    WaveDataProviderBoolean::WaveDataProviderBoolean(const std::string& saleaeDirectoryPath, const QList<WaveData*>& wdList, const char *ttable)
        : WaveDataProviderGroup(saleaeDirectoryPath,wdList), mTruthTable(nullptr)
    {
        mInputCount = wdList.size();
        int n = (mInputCount + 7) / 8;
        mTruthTable = new char[n];
        memcpy(mTruthTable, ttable, n);
    }

    WaveDataProviderBoolean::~WaveDataProviderBoolean()
    {
        if (mTruthTable) delete [] mTruthTable;
    }

    SaleaeDataTuple WaveDataProviderBoolean::convertToBoolean(SaleaeDataTuple sdt)
    {
        if (sdt.mValue >= 0)
        {
            int j = sdt.mValue / 8;
            int k = sdt.mValue % 8;
            sdt.mValue = (mTruthTable[j] & (1<<k)) ? 1 : 0;
        }
        return sdt;
    }

    SaleaeDataTuple WaveDataProviderBoolean::startValue(u64 t)
    {
        return convertToBoolean(WaveDataProviderGroup::startValue(t));
    }

    SaleaeDataTuple WaveDataProviderBoolean::nextPoint()
    {
        return convertToBoolean(WaveDataProviderGroup::nextPoint());
    }
}
