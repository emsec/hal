#pragma once
#include "hal_core/defines.h"
#include <map>
#include <unordered_map>
#include <functional>

namespace hal
{
    class SaleaeInputFile;

    class SaleaeParser
    {
        static std::string strim(std::string s);
        struct WaveFormFile {
            std::function<void(const void*obj, uint64_t, int)> callback;
            std::string name;
            SaleaeInputFile* file;
            int value;
            const void* obj;
        };
        std::multimap<uint64_t,WaveFormFile> mNextValueMap;
        std::unordered_map<std::string,std::string> mSaleaeAbbrevMap;
        std::string mCsvFilename;

        SaleaeInputFile* inputFileFactory(const std::string& abbrev) const;
    public:
        static uint64_t sTimeScaleFactor;
        u64 get_max_time() const;
        SaleaeParser(const std::string& filename);
        bool registerCallback(std::string& name, std::function<void(const void*,uint64_t, int)> callback, const void* obj);
        bool nextEvent();
    };
}
