#pragma once

#include <QMap>
#include <QPair>
#include "hal_core/defines.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/simulation_input.h"

namespace hal {

    class WaveDataProvider
    {
        bool mGroup;
        int mBits;
        int mValueBase;
    public:
        WaveDataProvider() : mGroup(false), mBits(1), mValueBase(16) {;}
        virtual ~WaveDataProvider() {;}
        virtual int startValue(u64 t) = 0;
        virtual SaleaeDataTuple nextPoint() = 0;
        bool isGroup() const { return mGroup; }
        int bits() const { return mBits; }
        int valueBase() const { return mValueBase; }
        void setGroup(bool grp, int bts, int base);
    };

    class WaveDataProviderMap : public WaveDataProvider
    {
        const QMap<u64,int>& mDataMap;
        QMap<u64,int>::const_iterator mIter;
    public:
        WaveDataProviderMap(const QMap<u64,int>& dmap) : mDataMap(dmap) {;}
        virtual int startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };

    class WaveDataProviderFile : public WaveDataProvider
    {
        SaleaeInputFile& mInputFile;
        SaleaeDataBuffer* mBuffer;
        u64 mIndex;
    public:
        WaveDataProviderFile(SaleaeInputFile& sif) : mInputFile(sif), mBuffer(nullptr), mIndex(0) {;}
        ~WaveDataProviderFile();
        virtual int startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };

    class WaveDataProviderClock : public WaveDataProvider
    {
        const SimulationInput::Clock& mClock;
        u64 mTransition;
        int valueForTransition() const;
    public:
        WaveDataProviderClock(const SimulationInput::Clock& clk) : mClock(clk), mTransition(0) {;}
        virtual int startValue(u64 t) override;
        virtual SaleaeDataTuple nextPoint() override;
    };
}
