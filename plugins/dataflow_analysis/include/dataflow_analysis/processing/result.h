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
#include "dataflow_analysis/processing/pass_collection.h"
#include "hal_core/defines.h"

#include <map>

namespace hal
{
    namespace dataflow
    {
        /**
         * The processing phase of dataflow analysis, which repeatedly applies grouping passes to produce candidate groupings.
         */
        namespace processing
        {
            /**
             * The outcome of the processing phase, i.e., all unique groupings that were produced and the pass combinations that led to them.
             */
            struct Result
            {
                /** All groupings that were produced, with duplicates removed. */
                std::vector<std::shared_ptr<Grouping>> unique_groupings;

                /** A map from each unique grouping to all sequences of passes that produce it. */
                std::map<std::shared_ptr<Grouping>, std::vector<std::vector<pass_id>>> pass_combinations_leading_to_grouping;

                /** A map from each sequence of passes to the grouping that it produces. */
                std::map<std::vector<pass_id>, std::shared_ptr<Grouping>> groupings;
            };

        }    // namespace processing
    }    // namespace dataflow
}    // namespace hal