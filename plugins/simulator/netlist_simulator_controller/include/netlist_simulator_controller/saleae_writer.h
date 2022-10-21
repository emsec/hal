// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
