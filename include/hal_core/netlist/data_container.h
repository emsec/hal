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
     *  Data structure for custom data associations.
     *
     * @ingroup netlist
     */
    class NETLIST_API DataContainer
    {
    public:
        /** constructor (= default) */
        DataContainer() = default;
        /** destructor (= default)  Needs to be virtual to access interface in subclasses via python. */
        virtual ~DataContainer() = default;

        /**
        * Sets a custom data entry.<br>
        * If it does not exist yet, it is added.
        *
        * @param[in] category - Key category
        * @param[in] key - Data key
        * @param[in] data_type - Data type of value
        * @param[in] value - Data value
        * @param[in] log_with_info_level - Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
        * @returns True on success.
        */
        bool set_data(const std::string& category, const std::string& key, const std::string& data_type, const std::string& value, const bool log_with_info_level = false);

        /**
         * Deletes custom data.
         *
         * @param[in] category - Category of key
         * @param[in] key - Data key
         * @param[in] log_with_info_level - Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
         * @returns True on success.
         */
        bool delete_data(const std::string& category, const std::string& key, const bool log_with_info_level = false);

        /**
         * Gets all stored data.
         *
         * @returns A map from ((1) category, (2) key) to ((1) type, (2) value)
         */
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> get_data() const;

        /**
         * Gets data specified by key and category.
         *
         * @param[in] category - Category of key
         * @param[in] key - Data key
         * @returns The tuple ((1) type, (2) value)
         */
        std::tuple<std::string, std::string> get_data_by_key(const std::string& category, const std::string& key) const;

        /**
         * Returns all data keys
         *
         * @returns A vector of tuples ((1) category, (2) key)
         */
        std::vector<std::tuple<std::string, std::string>> get_data_keys() const;

    protected:
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> m_data;
    };
}    // namespace hal
