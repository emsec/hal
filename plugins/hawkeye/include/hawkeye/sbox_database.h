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

/**
 * @file sbox_database.h 
 * @brief This file contains the S-box database class that holds and manages known cryptographic S-boxes up to 8 bits wide.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <map>
#include <vector>

namespace hal
{
    namespace hawkeye
    {
        /**
         * @class SBoxDatabase
         * @brief Database of known S-boxes.
         * 
         * This class holds and manages known S-boxes and allows to perform efficient S-box lookups in the database.
         */
        class SBoxDatabase
        {
        public:
            /**
             * @brief Construct an empty S-box database.
             */
            SBoxDatabase() = default;

            /**
             * @brief Construct an S-box database from the given S-boxes.
             * 
             * @param[in] sboxes - A map from S-box name to the respective S-box.
             */
            SBoxDatabase(const std::map<std::string, std::vector<u8>>& sboxes);

            /**
             * @brief Destruct the S-box database.
             */
            ~SBoxDatabase() = default;

            /**
             * @brief Construct an S-box database from file.
             * 
             * @param[in] file_path - The path from which to load the S-box database file.
             * @returns The S-box database on success, an error otherwise.
             */
            static Result<SBoxDatabase> from_file(const std::filesystem::path& file_path);

            /**
             * @brief Compute the linear representative of the given S-box.
             * 
             * @param[in] sbox - The S-box.
             * @returns The linear representative.
             */
            static std::vector<u8> compute_linear_representative(const std::vector<u8>& sbox);

            /**
             * @brief Add an S-box to the database.
             * 
             * @param[in] name - The name of the S-box.
             * @param[in] sbox - The S-box.
             * @returns Ok() on success, an error otherwise.
             */
            Result<std::monostate> add(const std::string& name, const std::vector<u8>& sbox);

            /**
             * @brief Add multiple S-boxes to the database.
             * 
             * @param[in] sboxes - A map from S-box name to the respective S-box.
             * @returns Ok() on success, an error otherwise.
             */
            Result<std::monostate> add(const std::map<std::string, std::vector<u8>>& sboxes);

            /**
             * @brief Load S-boxes from a file and add them to the existing database.
             * 
             * @param[in] file_path - The path from which to load the S-box database file.
             * @param[in] overwrite - Set `true` to overwrite existing database, `false` otherwise. Defaults to `false`.
             * @returns Ok() on success, an error otherwise.
             */
            Result<std::monostate> load(const std::filesystem::path& file_path, bool overwrite = false);

            /**
             * @brief Store the S-box database to a database file.
             * 
             * @param[in] file_path - The path to where to store the S-box database file.
             * @returns Ok() on success, an error otherwise.
             */
            Result<std::monostate> store(const std::filesystem::path& file_path) const;

            /**
             * @brief Attempt to look up an S-box in the database.
             * 
             * @param[in] sbox - The S-box to look for.
             * @returns Ok() and the S-box name on success, an error otherwise.
             */
            Result<std::string> lookup(const std::vector<u8>& sbox) const;

            /**
             * @brief Print the database.
             */
            void print() const;

        private:
            /**
             * Holds the S-boxes contained within the database.
             */
            std::map<u32, std::map<std::vector<u8>, std::vector<std::pair<std::string, u8>>>> m_data;
        };
    }    // namespace hawkeye

}    // namespace hal
