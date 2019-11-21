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

/**
 * @ingroup core
 */
class CORE_API program_arguments
{
public:
    /**
     * Constructor. Stores the command line arguments internally.
     *
     * @param[out] argc - Pointer to number of arguments.
     * @param[out] argv - Pointer to array of arguments.
     */
    program_arguments(int argc, const char** argv);

    /**
     * Constructor. Stores no command line arguments internally.
     */
    program_arguments();

    ~program_arguments() = default;

    /**
     * Copies the original command line arguments into the parameters.
     *
     * @param[out] argc - Pointer to number of arguments.
     * @param[out] argv - Pointer to array of arguments.
     */
    void get_original_arguments(int* argc, const char*** argv);

    /**
     * Returns all flags which were used to set options in the parsed arguments or programatically in order of appearance.<br>
     *
     * @returns All used flags to set options.
     */
    std::vector<std::string> get_set_options() const;

    /**
     * Checks whether an option was set.<br>
     * Any flag of the option works, even if one of the other flags was actually used to set the option.
     *
     * @param[in] flag - A flag of the option.
     * @returns True, if the user entered a flag for this option.
     */
    bool is_option_set(const std::string& flag) const;

    /**
     * Programatically set an option via a single flag and supply parameters.
     *
     * @param[in] flag - A flag of the option.
     * @param[in] parameters - The parameters for this flag.
     */
    void set_option(const std::string& flag, const std::vector<std::string>& parameters);

    /**
     * Programatically set an option (specifying a set of equivalent flags) and supply parameters.
     *
     * @param[in] flag - A single flag which is actually set.
     * @param[in] equivalent_flags - All flags for the same option.
     * @param[in] parameters - The parameters for this flag.
     * @returns True, if the flags were not found in multiple disjunct options already.
     */
    bool set_option(const std::string& flag, const std::set<std::string>& equivalent_flags, const std::vector<std::string>& parameters);

    /**
     * Returns the parameter of the option.<br>
     * If the option expects more than one parameter, use get_parameters().<br>
     * If the user did not supply a parameter, the default value is returned.
     *
     * @param[in] flag - A flag of the option. If the option was entered with multiple flags, any of these works.
     * @returns The parameter for the option.
     */
    std::string get_parameter(const std::string& flag) const;

    /**
     * Returns the parameters of the option.<br>
     * If the option expects only one parameter, use the shorthand get_parameter().<br>
     * If the user did not supply (all) parameters, the default values are returned in those places.
     *
     * @param[in] flag - A flag of the option. If the option was entered with multiple flags, any of these works.
     * @returns The parameters for the option.
     */
    std::vector<std::string> get_parameters(const std::string& flag) const;

private:
    int m_argc;
    const char** m_argv;

    std::map<std::set<std::string>, std::vector<std::string>> m_set_options;
    std::vector<std::string> m_given_flags;
};
