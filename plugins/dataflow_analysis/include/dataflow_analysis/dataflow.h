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

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/result.h"
#include "hal_core/utilities/result.h"

#include <unordered_set>

namespace hal
{
    class Gate;
    class Module;

    namespace dataflow
    {
        /**
         * Analyze the datapath to identify word-level registers in the given netlist.
         * 
         * @param[in] nl - The netlist.
         * @param[in] sizes - Register sizes that are expected to be found in the netlist. These sizes will be prioritized over others during analysis. Defaults to an empty vector.
         * @param[in] register_stage_identification - Set `true`to enable register stage identification during analysis, `false` otherwise. Defaults to `false`.
         * @param[in] known_groups - Registers that have been identified prior to dataflow analysis. Must be provided as a vector of registers with each register being represented as a vector of gate IDs. Defaults to an empty vector.
         * @param[in] bad_group_size - Minimum expected register size. Smaller registers will not be considered during analysis. Defults to `7`.
         * @returns Ok() and the dataflow analysis result on success, an error otherwise.
         */
        hal::Result<dataflow::Result>
            analyze(Netlist* nl, const std::vector<u32>& sizes = {}, bool register_stage_identification = false, const std::vector<std::vector<u32>>& known_groups = {}, const u32 bad_group_size = 7);
    }    // namespace dataflow
}    // namespace hal