#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "netlist_simulator_controller/saleae_directory.h"

namespace hal
{
    class SaleaeOutputFile;

    class SaleaeWriter
    {
        std::filesystem::path mDir;
        SaleaeDirectory mSaleaeDirectory;
        std::unordered_map<int,SaleaeOutputFile*> mDataFiles;
    public:
        SaleaeWriter(const std::string& filename);
        ~SaleaeWriter();
        SaleaeOutputFile* get_file_by_name(const std::string& name);
        SaleaeOutputFile* get_file_by_id(uint32_t id);
        SaleaeOutputFile* add_or_replace_waveform(const std::string& name, uint32_t id);
        void add_directory_entry(int inx, const std::string& name, uint32_t id);
    };
}
