//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"

#include <map>
#include <tuple>
#include <vector>

namespace hal
{
    /**
     * Container to hold data that is associated with an entity.
     *
     * @ingroup netlist
     */
    class NETLIST_API DataContainer
    {
    public:
        /**
         * Construct a new data container.
         */
        DataContainer() = default;

        virtual ~DataContainer() = default;

        /**
        * Add a data entry.<br>
        * May overwrite an existing entry.
        *
        * @param[in] category - The data category.
        * @param[in] key - The data key.
        * @param[in] data_type - The data type.
        * @param[in] value - The data value.
        * @param[in] log_with_info_level - Force explicit logging channel 'netlist' with log level 'info' to trace GUI events.
        * @returns True on success, false otherwise.
        */
        bool set_data(const std::string& category, const std::string& key, const std::string& data_type, const std::string& value, const bool log_with_info_level = false);

        /**
         * Get the data entry specified by the given category and key.
         *
         * @param[in] category - The data category.
         * @param[in] key - The data key.
         * @returns The tuple ((1) type, (2) value).
         */
        std::tuple<std::string, std::string> get_data(const std::string& category, const std::string& key) const;

        /**
         * Delete a data entry.
         *
         * @param[in] category - The data category.
         * @param[in] key - The data key.
         * @param[in] log_with_info_level - Force explicit logging channel 'netlist' with log level 'info' to trace GUI events.
         * @returns True on success, false otherwise.
         */
        bool delete_data(const std::string& category, const std::string& key, const bool log_with_info_level = false);

        /**
         * Get a map from ((1) category, (2) key) to ((1) type, (2) value) containing all stored data entries.
         *
         * @returns The stored data as a map.
         */
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> get_data_map() const;

        /**
         * Overwrite the existing data with a new map from ((1) category, (2) key) to ((1) type, (2) value).
         *
         * @param[in] map - The new data map.
         */
        void set_data_map(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& map);

    protected:
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> m_data;
    };
}    // namespace hal
