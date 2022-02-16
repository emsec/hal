#pragma once

#include <QMap>
#include <QPair>
#include "hal_core/defines.h"
#include "netlist_simulator_controller/saleae_file.h"

namespace hal {

    class WaveDataProvider
    {
    public:
        WaveDataProvider() {;}
        virtual ~WaveDataProvider() {;}
        virtual int startValue(u64 t) = 0;
        virtual SaleaeDataTuple nextPoint() = 0;
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
}
