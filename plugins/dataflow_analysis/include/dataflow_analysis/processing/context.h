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

#include "dataflow_analysis/processing/pass_collection.h"
#include "dataflow_analysis/processing/result.h"
#include "hal_core/defines.h"

#include <map>
#include <shared_mutex>

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct PipelineConfiguration;
        struct NetlistAbstraction;

        namespace processing
        {
            /**
             * The state that is carried over between the passes of the processing phase of dataflow analysis.
             */
            struct Context
            {
                /* pass process */

                /** Set `true` once no further pass can be applied, `false` otherwise. */
                bool done        = false;

                /** Set `true` once the configured number of pass layers has been reached, `false` otherwise. */
                bool end_reached = false;

                /** The pass layer that is currently being processed. */
                u32 phase        = 0;

                /** The number of passes that have been scheduled so far. */
                u32 pass_counter    = 0;

                /** The number of passes that have completed so far. */
                u32 finished_passes = 0;

                /** The number of iterations that the processing phase has performed. */
                u32 num_iterations  = 0;

                // std::shared_ptr<grouping> main_state;
                // std::shared_ptr<grouping> control_state;

                /* helpers */

                /** The passes that are currently being executed, each paired with the grouping it is applied to. */
                std::vector<std::pair<std::shared_ptr<Grouping>, PassConfiguration>> current_passes;

                /** A map from an input grouping and a pass to the grouping that the pass produced. */
                std::map<std::pair<std::shared_ptr<Grouping>, pass_id>, std::shared_ptr<Grouping>> pass_outcome;

                /** The groupings produced in this iteration that have not been seen before. */
                std::vector<std::tuple<std::shared_ptr<Grouping>, pass_id, std::shared_ptr<Grouping>>> new_unique_groupings;

                /** The groupings produced in this iteration that had already been seen before. */
                std::vector<std::tuple<std::shared_ptr<Grouping>, pass_id, std::shared_ptr<Grouping>>> new_recurring_results;

                /* progress printing */

                /** Guards the progress output, which is written from several worker threads. */
                std::mutex progress_mutex;

                /* result */

                /** Guards `result`, which is filled by several worker threads. */
                std::mutex result_mutex;

                /** The result that has been accumulated so far. */
                processing::Result result;
            };

        }    // namespace processing
    }    // namespace dataflow
}    // namespace hal