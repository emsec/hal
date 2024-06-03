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
 * @brief This file contains the function for HAWKEYE's candidate search as well as a struct for configuring the search algorithm.
 */

#pragma once

#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/result.h"
#include "hawkeye/register_candidate.h"

#include <vector>

namespace hal
{
    class Netlist;
    class Gate;

    namespace hawkeye
    {
        /**
         * @struct DetectionConfiguration
         * @brief Configuration to set up the register candidate search.
         * 
         * This struct holds important parameters that configure the candidate search of HAWKEYE.
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
             * @brief Minimum number of flip-flops for a register candidate to be created.
             */
            u32 min_register_size = 10;
        };

        /**
         * @brief Attempt to locate candidates for symmetric cryptographic implementations within a gate-level netlist.
         * 
         * Search operates only on an abstraction of the netlist that contains only flip-flops as nodes and connections through combinational logic as edges.
         * The algorithm computes the k-neighborhood of each flip-flop for `k = 1, ..., config.timeout` and stops when the neighborhood size saturates.
         * Depending on the `config`, additional criteria are used to narrow down the search space, see `DetectionConfiguration::Control` and `DetectionConfiguration::Components` for details.
         * When the neighborhood size saturates, a register candidate is created if the last neighborhood size is larger than `config.min_register_size`.
         * After the candidates have been identified, they are reduced further to produce the final set of register candidates.
         * To this end, large candidates that fully contain a smaller candidate and candidates that are smaller than `min_state_size` are discarded.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] config - The configurations of the detection approaches to be executed one after another on each start flip-flop.
         * @param[in] min_state_size - The minimum size of a register candidate to be considered a cryptographic state register. Defaults to `40`.
         * @param[in] start_ffs - The flip-flops to analyze. Defaults to an empty vector, i.e., all flip-flops in the netlist will be analyzed.
         * @returns Ok() and a vector of candidates on success, an error otherwise.
         */
        Result<std::vector<RegisterCandidate>> detect_candidates(Netlist* nl, const std::vector<DetectionConfiguration>& configs, u32 min_state_size = 40, const std::vector<Gate*>& start_ffs = {});
    }    // namespace hawkeye

    template<>
    std::map<hawkeye::DetectionConfiguration::Control, std::string> EnumStrings<hawkeye::DetectionConfiguration::Control>::data;

    template<>
    std::map<hawkeye::DetectionConfiguration::Components, std::string> EnumStrings<hawkeye::DetectionConfiguration::Components>::data;
}    // namespace hal