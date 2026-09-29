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

#include "hal_core/defines.h"
#include "hal_core/netlist/parameter.h"
#include "hal_core/utilities/result.h"

#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
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
         * Check whether two data containers are equal.
         *
         * @param[in] other - The data container to compare against.
         * @returns `true` if both data containers are equal, `false` otherwise.
         */
        bool operator==(const DataContainer& other) const;

        /**
         * Check whether two data containers are unequal.
         *
         * @param[in] other - The data container to compare against.
         * @returns `true` if both data containers are unequal, `false` otherwise.
         */
        bool operator!=(const DataContainer& other) const;

        /**
        * Add a data entry.<br>
        * May overwrite an existing entry.
        *
        * @param[in] category - The data category.
        * @param[in] key - The data key.
        * @param[in] data_type - The data type.
        * @param[in] value - The data value.
        * @param[in] log_with_info_level - Force explicit logging channel 'netlist' with log level 'info' to trace GUI events.
        * @returns `true` on success, `false` otherwise.
        */
        bool set_data(const std::string& category, const std::string& key, const std::string& data_type, const std::string& value, const bool log_with_info_level = false);

        /**
         * Determine whether an entry of given category and key exists.
         * 
         * @param[in] category - The data category.
         * @param[in] key - The data key.
         * @returns `true` if the entry exists, `false` otherwise.
         */
        bool has_data(const std::string& category, const std::string& key) const;

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
         * @returns `true` on success, `false` otherwise.
         */
        bool delete_data(const std::string& category, const std::string& key, const bool log_with_info_level = false);

        /**
         * Get a map from ((1) category, (2) key) to ((1) type, (2) value) containing all stored data entries.
         *
         * @returns The stored data as a map.
         */
        const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& get_data_map() const;

        /**
         * Overwrite the existing data with a new map from ((1) category, (2) key) to ((1) type, (2) value).
         *
         * @param[in] map - The new data map.
         */
        void set_data_map(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& map);

        /** The key of a typed value: its source and its name. */
        using ParameterKey = std::pair<Parameter::Source, std::string>;

        /**
         * Map a category of the legacy data map to the source of the typed store: `generic` to `Generic`,
         * `attribute` to `Attribute`. Gate libraries name INIT and location data by such a category.
         *
         * @param[in] category - The category.
         * @returns The source, or nothing for any other category.
         */
        static std::optional<Parameter::Source> source_from_category(const std::string& category);

        /**
         * Set (or overwrite) the value of a typed parameter as a string. The declaration's source says whether the
         * value is a generic or an attribute; a generic and an attribute may share a name. The value is validated and
         * normalized before being stored. Any existing entry with the same source and name is replaced.
         *
         * @param[in] param - The parameter declaration.
         * @param[in] value - The value to store as a string.
         * @returns OK() on success, an error otherwise.
         */
        Result<std::monostate> set_parameter(const Parameter& param, const std::string& value);

        /**
         * Get the value of a typed parameter.
         *
         * @param[in] name - The parameter name.
         * @param[in] source - The source, generic or attribute.
         * @returns OK() with the value string on success, an error if the parameter does not exist.
         */
        Result<std::string> get_parameter_value(const std::string& name, Parameter::Source source = Parameter::Source::Generic) const;

        /**
         * Get the value of a typed parameter whose declaration matches the given one.
         *
         * @param[in] param - The parameter declaration.
         * @returns OK() with the value string on success, an error if there is no parameter with that source and name or its declaration differs.
         */
        Result<std::string> get_parameter_value(const Parameter& param) const;

        /**
         * Get the full declaration of a typed parameter.
         *
         * @param[in] name - The parameter name.
         * @param[in] source - The source, generic or attribute.
         * @returns OK() with the declaration on success, an error if the parameter does not exist.
         */
        Result<Parameter> get_parameter_declaration(const std::string& name, Parameter::Source source = Parameter::Source::Generic) const;

        /**
         * Check whether a parameter with the given declaration exists.
         *
         * @param[in] param - The parameter declaration.
         * @returns `true` if a parameter with that source and name exists and its declaration matches, `false` otherwise.
         */
        bool has_parameter(const Parameter& param) const;

        /**
         * Check whether a parameter with the given name exists.
         *
         * @param[in] name - The parameter name.
         * @param[in] source - The source, generic or attribute.
         * @returns `true` if the parameter exists, `false` otherwise.
         */
        bool has_parameter(const std::string& name, Parameter::Source source = Parameter::Source::Generic) const;

        /**
         * Delete a stored parameter, if any.
         *
         * @param[in] name - The parameter name.
         * @param[in] source - The source, generic or attribute.
         * @returns `true` if a parameter was deleted, `false` otherwise.
         */
        bool delete_parameter(const std::string& name, Parameter::Source source = Parameter::Source::Generic);

        /**
         * Get all stored typed values as a map from `(source, name)` to `(declaration, value)`.
         *
         * @returns The parameter map.
         */
        const std::map<ParameterKey, std::pair<Parameter, std::string>>& get_parameters() const;

        /**
         * Get the stored typed values of one source as a map from `name` to `(declaration, value)`.
         *
         * @param[in] source - The source, generic or attribute.
         * @returns The parameter map.
         */
        std::map<std::string, std::pair<Parameter, std::string>> get_parameters(Parameter::Source source) const;

    protected:
        std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> m_data;
        std::map<ParameterKey, std::pair<Parameter, std::string>> m_parameters;
    };
}    // namespace hal
