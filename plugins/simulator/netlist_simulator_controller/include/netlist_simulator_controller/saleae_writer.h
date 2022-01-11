#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

namespace hal
{
    class SaleaeOutputFile;

    class SaleaeWriter
    {
        std::filesystem::path mDir;
        std::ofstream* mCsvFile;
        std::vector<SaleaeOutputFile*> mDataFiles;
    public:
        SaleaeWriter(const std::string& filename);
        ~SaleaeWriter();
        SaleaeOutputFile* get_file_by_name(const std::string& name);
        SaleaeOutputFile* get_file_by_id(uint32_t id);
        SaleaeOutputFile* addWaveForm(const std::string& name);
    };
}
