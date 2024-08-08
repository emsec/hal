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
 * @file module_identification.h
 * @brief This file contains the function declarations for the Module Identification plugin in hal.
 */

#pragma once

#include "hal_core/utilities/result.h"

namespace hal
{
    class Gate;

    namespace module_identification
    {
        class Configuration;
        class Result;

        /**
         * @brief Perform a full run of the module identification process on the given netlist with the provided configuration.
         * 
         * This function executes the complete module identification process on the specified netlist. It uses the provided configuration to guide the identification process and returns a result object containing information about all analyzed candidates.
         * 
         * @param[in] config - The configuration to guide the identification process.
         * @returns OK() and the result of the run containing all computed results and options for further processing on success, an error otherwise.
         */
        hal::Result<Result> execute(const Configuration& config);

        /**
         * @brief Perform a module identification run on the specified gates with the provided configuration.
         * 
         * This function executes the module identification process on a specific set of gates. It uses the provided configuration to guide the identification process and returns a result object containing information about all analyzed candidates.
         * 
         * @param[in] gates - The gates to be analyzed.
         * @param[in] config - The configuration to guide the identification process.
         * @returns OK() and the result of the run containing all computed results and options for further processing on success, an error otherwise.
         */
        hal::Result<Result> execute_on_gates(const std::vector<Gate*>& gates, const Configuration& config);

    }    // namespace module_identification
}    // namespace hal
