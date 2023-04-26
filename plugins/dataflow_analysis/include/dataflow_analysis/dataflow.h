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
    class Grouping;

    namespace dataflow
    {
        /**
         * Holds the configuration of a dataflow analysis run.
         */
        struct Configuration
        {
            /**
             * Minimum size of a group. Smaller groups will be penalized during analysis. Defaults to 8.
             */
            u32 min_group_size = 8;

            /**
             * Expected group sizes. Groups of these sizes will be prioritized. Defaults to an empty vector.
             */
            std::vector<u32> expected_sizes = {};

            /**
             * Already identified groups of sequential gates as a vector of groups with each group being a vector of gate IDs. Defaults to an empty vector.
             */
            std::vector<std::vector<u32>> known_groups = {};

            /**
             * Enable register stage identification as part of dataflow analysis. Defaults to `false`.
             */
            bool enable_register_stages = false;

            /**
             * Set the minimum size of a group. Smaller groups will be penalized during analysis.
             * 
             * @param[in] size - The minimum group size.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_min_group_size(u32 size);

            /**
             * Set the expected group sizes. Groups of these sizes will be prioritized.
             * 
             * @param[in] sizes - The expected group sizes.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_expected_sizes(const std::vector<u32>& sizes);

            /**
             * Set already identified groups of sequential gates as a vector of groups with each group being a module.
             * 
             * @param[in] groups - A vector of groups.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<Module*>& groups);

            /**
             * Set already identified groups of sequential gates as a vector of groups with each group being a grouping.
             * 
             * @param[in] groups - A vector of groups.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<hal::Grouping*>& groups);

            /**
             * Set already identified groups of sequential gates as a vector of groups with each group being a vector of gates.
             * 
             * @param[in] groups - A vector of groups.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<std::vector<Gate*>>& groups);

            /**
             * Set already identified groups of sequential gates as a vector of groups with each group being a vector of gate IDs.
             * 
             * @param[in] groups - A vector of group.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<std::vector<u32>>& groups);

            /**
             * Enable register stage identification as part of dataflow analysis.
             * 
             * @param[in] enable - Set `true` to enable register stage identification, `false` otherwise. Defaults to `true`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_register_stage_identification(bool enable = true);
        };

        /**
         * Analyze the datapath to identify word-level registers in the given netlist.
         * 
         * @param[in] nl - The netlist.
         * @param[in] config - The dataflow analysis configuration.
         * @returns Ok() and the dataflow analysis result on success, an error otherwise.
         */
        hal::Result<dataflow::Result> analyze(Netlist* nl, const Configuration& config = Configuration());
    }    // namespace dataflow
}    // namespace hal