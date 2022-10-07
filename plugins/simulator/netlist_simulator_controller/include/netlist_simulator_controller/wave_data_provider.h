//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include <QMap>
#include <QPair>
#include "hal_core/defines.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_parser.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_group_value.h"
#include "netlist_simulator_controller/wave_data.h"

namespace hal {

    class WaveDataTimeframe;
    class WaveData;

    class WaveDataProvider
    {
        WaveData::NetType mWaveType;
        int mBits;
        int mValueBase;
    public:
        WaveDataProvider() : mWaveType(WaveData::RegularNet), mBits(1), mValueBase(16) {;}
        virtual ~WaveDataProvider() {;}
        virtual SaleaeDataTuple startValue(u64 t) = 0;
        virtual SaleaeDataTuple nextPoint() = 0;
        bool isGroup()   const { return mWaveType == WaveData::NetGroup; }
        bool isBoolean() const { return mWaveType == WaveData::BooleanNet; }
        bool isTrigger() const { return mWaveType == WaveData::TriggerTime; }
        int bits() const { return mBits; }
        int valueBase() const { return mValueBase; }
        void setWaveType(WaveData::NetType type, int bts=1, int base=16);
    };

    class WaveDataProviderMap : public WaveDataProvider
    {
        const QMap<u64,int>& mDataMap;
        QMap<u64,int>::const_iterator mIter;
    public:
        WaveDataProviderMap(const QMap<u64,int>& dmap) : mDataMap(dmap) {;}
        virtual SaleaeDataTuple startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };

    class WaveDataProviderFile : public WaveDataProvider
    {
    public:
        enum StoreData { Off, Recording, Complete, Failed};
        SaleaeInputFile& mInputFile;
        SaleaeDataBuffer* mBuffer;
        u64 mIndex;
        const WaveDataTimeframe& mTimeframe;
        QMap<u64,int> mDataMap;
        StoreData mStoreData;

        bool isRecording() const { return mStoreData == Recording; }
        void storeCurrentDatapoint();
    public:
        WaveDataProviderFile(SaleaeInputFile& sif, const WaveDataTimeframe& tframe) : mInputFile(sif), mBuffer(nullptr), mIndex(0),
            mTimeframe(tframe), mStoreData(Off) {;}
        ~WaveDataProviderFile();
        virtual SaleaeDataTuple startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;

        StoreData storeDataState() const { return mStoreData; }
        const QMap<u64,int>& dataMap() const { return mDataMap; }
    };

    class WaveDataProviderClock : public WaveDataProvider
    {
        const SimulationInput::Clock& mClock;
        u64 mTransition;
        int valueForTransition() const;
    public:
        WaveDataProviderClock(const SimulationInput::Clock& clk) : mClock(clk), mTransition(0) {;}
        virtual SaleaeDataTuple startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };

    class WaveDataProviderGroup : public WaveDataProvider
    {
        SaleaeParser mParser;
        u32* mBitMask;
        u64 mCurrentTime;
        u64 mSampleTime;
        WaveGroupValue mLastValue;
        WaveGroupValue mNextValue;
        int mSampleValue;
        bool mValuePending;
        bool mEventReady;

        bool nextEventReady();
    public:
        WaveDataProviderGroup(const std::string& saleaeDirectoryPath, const QList<WaveData*>& wdList);
        virtual ~WaveDataProviderGroup();
        virtual SaleaeDataTuple startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };

    class WaveDataProviderBoolean : public WaveDataProviderGroup
    {
        int mInputCount;
        char* mTruthTable;
        SaleaeDataTuple convertToBoolean(SaleaeDataTuple sdt);
    public:
        WaveDataProviderBoolean(const std::string& saleaeDirectoryPath, const QList<WaveData*>& wdList, const char* ttable);
        virtual ~WaveDataProviderBoolean();
        virtual SaleaeDataTuple startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };

    class WaveDataProviderTrigger : public WaveDataProvider
    {
        SaleaeParser mParser;
        WaveData* mFilter;
        int* mTransitionToValue;
        u64 mCurrentTime;
        bool mCurrentTrigger;
        qint64 mReportedTime;
    public:
        WaveDataProviderTrigger(const std::string& saleaeDirectoryPath, const QList<WaveData*>& wdList, const QList<int>& toValue, WaveData* filter=nullptr);
        virtual ~WaveDataProviderTrigger();
        virtual SaleaeDataTuple startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };
}
