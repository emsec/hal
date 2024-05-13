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

#include "hal_core/utilities/result.h"
#include "hawkeye/candidate.h"

namespace hal
{
    class Netlist;

    namespace hawkeye
    {
        struct DetectionConfiguration
        {
            enum class Control
            {
                CHECK_FF,
                CHECK_TYPE,
                CHECK_PINS,
                CHECK_NETS
            } control = Control::CHECK_NETS;

            enum class Components
            {
                NONE,
                CHECK_SCC
            } components = Components::NONE;

            u32 timeout           = 10;
            u32 min_register_size = 10;
        };

        /**
         * TODO description
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] config - The configurations of the detection approaches to be executed one after another on each start flip-flop.
         * @param[in] min_state_size - The minimum size of a register candidate to be considered a cryptographic state register. Defaults to `40`.
         * @param[in] start_ffs - The flip-flops to analyze. Defaults to an empty vector, i.e., all flip-flops in the netlist will be analyzed.
         * @returns Ok() and a vector of candidates on success, an error otherwise.
         */
        Result<std::vector<Candidate>> detect_candidates(Netlist* nl, const std::vector<DetectionConfiguration>& configs, u32 min_state_size = 40, const std::vector<Gate*>& start_ffs = {});
    }    // namespace hawkeye
}    // namespace hal