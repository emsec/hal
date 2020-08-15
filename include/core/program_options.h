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

#include "def.h"

#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace hal
{
    class ProgramArguments;

    /**
    * @ingroup core
    */
    class CORE_API ProgramOptions
    {
    public:
        /// constant to specify that a parameter is required and does not have a default value.
        static const std::string A_REQUIRED_PARAMETER;

        /**
         * Constructor, optionally takes a name for grouping in get_options_string()
         *
         * @param[in] name - The name of this group of program options.
         */
        ProgramOptions(const std::string& name = "");

        ~ProgramOptions() = default;

        /**
         * Parses the command line arguments into the internal structure.
         *
         * @param[in] argc - Number of arguments.
         * @param[in] argv - Array of arguments.
         * @returns The parsed arguments.
         */
        ProgramArguments parse(int argc, const char* argv[]);

        /**
         * Returns the command line arguments which could not be parsed.<br>
         * Only valid after parse() was called.
         *
         * @returns A vector containing all strings which could not be parsed.
         */
        std::vector<std::string> get_unknown_arguments();

        /**
         * Checks whether a flag is already registered for an option.<br>
         * No flag can be registered twice.
         *
         * @param[in] flag - The flag to check.
         * @returns True if the flag is already registered.
         */
        bool is_registered(const std::string& flag) const;

        /**
         * Adds a new option with a single flag.<br>
         * The size of \p parameters is the number of parameters this option gets.<br>
         * Use ProgramOptions::A_REQUIRED_PARAMETER to set a parameter as required.<br>
         * The values in \p parameters contain default values, which are returned by
         * get_parameter_list() / get_parameter() if the user did not supply parameters himself.
         *
         * @param[in] flag - The flag activating the option.
         * @param[in] description - A description of the option.
         * @param[in] parameters - A list of default values for all parameters. [optional]
         * @returns True on success.
         */
        bool add(const std::string& flag, const std::string& description, const std::initializer_list<std::string>& parameters = {});

        /**
         * Adds a new option with a multiple flags.<br>
         * The size of \p parameters is the number of parameters this option gets.<br>
         * Use ProgramOptions::A_REQUIRED_PARAMETER to set a parameter as required.<br>
         * The values in \p parameters contain default values, which are returned by
         * get_parameter_list() / get_parameter() if the user did not supply parameters himself.
         *
         * @param[in] flags - The flags activating the option.
         * @param[in] description - A description of the option.
         * @param[in] parameters - A list of default values for all parameters. [optional]
         * @returns True on success.
         */
        bool add(const std::initializer_list<std::string>& flags, const std::string& description, const std::initializer_list<std::string>& parameters = {});

        /**
         * Adds another set of options.<br>
         * A category can be supplied for grouping in the get_options_string() function.
         *
         * @param[in] other_options - The set of options to add.
         * @param[in] category - A category for the added options. [optional]
         * @returns True on success.
         */
        bool add(const ProgramOptions& other_options, const std::string& category = "");

        /**
         * Returns a nicely formatted string of all options and description.<br>
         * Includes categorys of added ProgramOptions for grouping.<br>
         * Useful for "usage" messages.
         *
         * @returns The formatted string.
         */
        std::string get_options_string() const;

        /**
         * Returns all options' flags and their description.
         *
         * @returns A vector of ( (1) a set with all flags of the option, (2) the description of the option).
         */
        std::vector<std::tuple<std::set<std::string>, std::string>> get_options() const;

    private:
        struct Option
        {
            std::string description;
            std::vector<std::string> parameters;
            std::set<std::string> flags;
        };

        std::string m_name;

        std::vector<Option> m_options;

        std::map<std::string, std::vector<ProgramOptions>> m_suboptions;

        std::vector<std::string> m_unknown_options;

        // returns all options, including those of added ProgramOptions objects
        std::vector<const Option*> get_all_options() const;

        // resets all options, including those of added ProgramOptions objects
        void reset_all_options();

        // resets a single option
        void reset_option(Option*);

        // returns the length of the longest formatted string containing the flags
        size_t get_flag_length() const;

        // returns the formatted string and starts the always after fill_length characters (space filled)
        std::string get_options_string_internal(size_t fill_length, size_t max_line_width) const;
    };
}    // namespace hal
