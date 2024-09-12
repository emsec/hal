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
 * @file dataflow.h
 * @brief This file contains the function that analyses the dataflow of a gate-level netlist.
 */

#pragma once

#include "dataflow_analysis/api/configuration.h"
#include "dataflow_analysis/api/result.h"
#include "dataflow_analysis/common/grouping.h"
#include "hal_core/utilities/result.h"

#include <unordered_set>

namespace hal
{
    class Netlist;

    namespace dataflow
    {
        /**
         * @brief Analyze the gate-level netlist to identify word-level structures such as registers.
         * 
         * Reconstructs word-level structures such as registers based on properties such as the control inputs of, e.g., their flip-flops and common successors/predecessors.
         * Operates on an abstraction of the netlist that, e.g., contains only flip-flops and connections between two flip-flops only if they are connected through combinational logic.
         * 
         * @param[in] config - The dataflow analysis configuration.
         * @returns Ok() and the dataflow analysis result on success, an error otherwise.
         */
        hal::Result<dataflow::Result> analyze(const Configuration& config);
    }    // namespace dataflow
}    // namespace hal