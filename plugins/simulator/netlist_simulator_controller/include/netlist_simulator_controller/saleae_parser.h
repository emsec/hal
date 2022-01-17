#pragma once
#include "hal_core/defines.h"
#include <map>
#include <unordered_map>
#include <functional>
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/saleae_file.h"

namespace hal
{
    class SaleaeInputFile;
    class Net;

    class SaleaeParser
    {
        SaleaeDirectory mSaleaeDirectory;
        static std::string strim(std::string s);
        struct DataFileHandle {
            std::function<void(const void*obj, uint64_t, int)> callback;
            SaleaeInputFile* file;
            int value;
            const void* obj;
        };
        std::multimap<uint64_t,DataFileHandle> mNextValueMap;

    public:
        static uint64_t sTimeScaleFactor;
        u64 get_max_time() const;
        SaleaeParser(const std::string& filename);
        bool registerCallback(const Net* net, std::function<void(const void*,uint64_t, int)> callback, const void* obj);
        bool nextEvent();
        SaleaeDataBuffer get_waveform_by_net(const Net* net) const;
    };
}
