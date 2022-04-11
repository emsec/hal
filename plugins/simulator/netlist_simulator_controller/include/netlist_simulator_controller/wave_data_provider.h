#pragma once

#include <QMap>
#include <QPair>
#include "hal_core/defines.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_parser.h"
#include "netlist_simulator_controller/simulation_input.h"
#include "netlist_simulator_controller/wave_group_value.h"

namespace hal {

    class WaveDataTimeframe;
    class WaveData;

    class WaveDataProvider
    {
        bool mGroup;
        bool mBoolean;
        int mBits;
        int mValueBase;
    public:
        WaveDataProvider() : mGroup(false), mBoolean(false), mBits(1), mValueBase(16) {;}
        virtual ~WaveDataProvider() {;}
        virtual SaleaeDataTuple startValue(u64 t) = 0;
        virtual SaleaeDataTuple nextPoint() = 0;
        bool isGroup() const { return mGroup; }
        bool isBoolean() const { return mBoolean; }
        int bits() const { return mBits; }
        int valueBase() const { return mValueBase; }
        void setGroup(bool grp, int bts, int base);
        void setBoolean(bool bl, int bts);
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
}
