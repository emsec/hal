#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "netlist_simulator_controller/saleae_directory.h"

namespace hal
{
    class SaleaeOutputFile;

    /**
     * @brief The SaleaeWriter class is a helper class which provides utility methods when importing data to SALEAE database.
     * The SaleaeWriter class is used for importing VCD, CSD, or external SALEAE files.
     */
    class SaleaeWriter
    {
        std::filesystem::path mDir;
        SaleaeDirectory mSaleaeDirectory;
        std::unordered_map<int,SaleaeOutputFile*> mDataFiles;
    public:
        /**
         * Constructor for SaleaeWriter
         * @param filename Full path and filename of SALEAE directory file
         */
        SaleaeWriter(const std::string& filename);

        /// Destructor closes all open binary files updating their file header. It also updates SALEAE directory
        ~SaleaeWriter();

        /// Create new empty output file. Will delete existing binary file if directory links already a data file with net entry.
        SaleaeOutputFile* add_or_replace_waveform(const std::string& name, uint32_t id);

        /// Adds directory entry for net/waveform with binary file index inx
        void add_directory_entry(int inx, const std::string& name, uint32_t id);
    };
}
