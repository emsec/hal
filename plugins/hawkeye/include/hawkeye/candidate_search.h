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
 * @file candidate_search.h
 * @brief This file contains the struct for configuring HAWKEYE's candidate search, see `CipherCandidate::detect`.
 */

#pragma once

#include "hal_core/utilities/enums.h"

#include <map>
#include <string>
#include <vector>

namespace hal
{
    namespace hawkeye
    {
        /**
         * @struct DetectionConfiguration
         * @brief Configuration to set up the candidate search.
         * 
         * This struct holds important parameters that configure the candidate search of HAWKEYE, see `CipherCandidate::detect`.
         */
        struct DetectionConfiguration
        {
            /**
             * @enum Control
             * @brief Checks to be performed on flip-flop control inputs during candidate search.
             * 
             * This enum specifies the checks that are to be performed on the flip-flops of the netlist to determine whether there should be an edge between two flip-flops or not.
             */
            enum class Control
            {
                /**
                 * @brief If two flip-flops `ff1` and `ff2` are connected through combinational logic, an edge is added such that `(ff1,ff2)` is part of the graph.
                 */
                CHECK_FF,

                /**
                 * @brief If two flip-flops `ff1` and `ff2` are connected through combinational logic and are of the same gate type, an edge is added such that `(ff1,ff2)` is part of the graph.
                 */
                CHECK_TYPE,

                /**
                 * @brief If two flip-flops `ff1` and `ff2` are connected through combinational logic and are controlled through the same input pins, an edge is added such that `(ff1,ff2)` is part of the graph.
                 */
                CHECK_PINS,

                /**
                 * @brief If two flip-flops `ff1` and `ff2` are connected through combinational logic and are controlled through the same input nets, an edge is added such that `(ff1,ff2)` is part of the graph.
                 */
                CHECK_NETS
            } control = Control::CHECK_NETS;

            /**
             * @enum Components
             * @brief Determines whether to use SCC detection as part of neighborhood discovery.
             * 
             * This enum specifies whether SCC detection should be used to refine the results of neighborhood discovery. If SCC detection is used, the exploration only stops if the size of the largest discovered SCC saturates. Specifically, it does no longer require the size of the entire neighborhood to saturate.
             */
            enum class Components
            {
                /**
                 * @brief Do not use SCC detection and instead resort to the simple neighborhood discovery algorithm.
                 */
                NONE,

                /**
                 * @brief Use SCC detection within the currently explored neighborhood of a start flip-flop. 
                 */
                CHECK_SCC
            } components = Components::NONE;

            /**
             * @brief A vector of a vector of gate types that are treated as identical types by the candidate search, i.e., when checking equality of the types of two gates that are different but declared equivalent, `true` is returned.
             */
            std::vector<std::vector<std::string>> equivalent_types;

            /**
             * @brief Neighborhood discovery iteration timeout.
             */
            u32 timeout = 10;

            /**
             * @brief Minimum number of flip-flops of a register for a candidate to be created from it.
             */
            u32 min_register_size = 10;
        };
    }    // namespace hawkeye

    template<>
    std::map<hawkeye::DetectionConfiguration::Control, std::string> EnumStrings<hawkeye::DetectionConfiguration::Control>::data;

    template<>
    std::map<hawkeye::DetectionConfiguration::Components, std::string> EnumStrings<hawkeye::DetectionConfiguration::Components>::data;
}    // namespace hal