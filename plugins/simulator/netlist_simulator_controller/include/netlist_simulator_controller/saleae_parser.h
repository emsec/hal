#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#include <string>
#ifdef STANDALONE_PARSER
#include "saleae_directory.h"
#include "saleae_file.h"
#else
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/saleae_file.h"
#endif

namespace hal
{
    class SaleaeInputFile;

#ifdef STANDALONE_PARSER
    class Net
    {
      uint32_t mId;
      std::string mName;
    public:
      Net(const std::string n, uint32_t i=0) : mId(i), mName(n) {;}
      uint32_t get_id() const { return mId; }
      std::string get_name() const { return mName; }
    };
#else
    class Net;
#endif
  
    class SaleaeParser
    {
        SaleaeDirectory mSaleaeDirectory;
        static std::string strim(std::string s);
        struct DataFileHandle {
            std::function<void(void*obj, uint64_t, int)> callback;
            SaleaeInputFile* file;
            int value;
            void* obj;
        };
        std::multimap<uint64_t,DataFileHandle> mNextValueMap;

    public:
        SaleaeParser(const std::string& filename);
        uint64_t get_max_time() const;
        bool register_callback(const Net* net, std::function<void(void*,uint64_t, int)> callback, void* obj);
        bool next_event();
        SaleaeDataBuffer get_waveform_by_net(const Net* net) const;

        SaleaeDirectory get_directory() const { return mSaleaeDirectory; }
        static uint64_t sTimeScaleFactor;
    };
}
