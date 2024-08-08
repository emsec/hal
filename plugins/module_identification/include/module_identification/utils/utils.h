// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All rights reserved.
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
 * @file module_identification_helpers.h
 * @brief This file contains helper functions for module identification in the HAL framework.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <set>
#include <vector>

namespace hal
{
    class Net;
    class Gate;

    namespace module_identification
    {
        /**
         * @brief Find neighboring registers connected to a given net.
         *
         * @param[in] n - The net to analyze.
         * @param[in] registers - A vector of vectors containing sets of gates representing registers.
         * @returns A set of indices representing the neighboring registers.
         */
        std::set<u32> find_neighboring_registers(const Net* n, const std::vector<std::vector<Gate*>>& registers);

        /**
         * @brief Find neighboring registers connected to a list of nets.
         *
         * @param[in] nets - A vector of nets to analyze.
         * @param[in] registers - A vector of vectors containing sets of gates representing registers.
         * @returns A set of indices representing the neighboring registers.
         */
        std::set<u32> find_neighboring_registers(const std::vector<Net*> nets, const std::vector<std::vector<Gate*>>& registers);

        /**
         * @brief Reorder commutative operands based on a permutation cache.
         *
         * This function reorders the operands of a commutative operation based on the provided permutation cache. This can optimize the operand order for specific calculations or verifications.
         *
         * @param[in] operands - A vector of vectors containing nets representing operands.
         * @param[in] registers - A vector of vectors containing gates used for permutation.
         * @param[in] permute_start_index - The starting index for permutations (default is 0).
         * @returns A reordered vector of vectors containing the operands.
         */
        std::vector<std::vector<Net*>>
            reorder_commutative_operands(const std::vector<std::vector<Net*>>& operands, const std::vector<std::vector<Gate*>>& registers, const u32 permute_start_index = 0);

        /**
         * @brief Get input nets from a list of gates.
         *
         * This function retrieves the input nets connected to a specified list of gates.
         *
         * @param[in] gates - A vector of gates to analyze.
         * @returns A vector of nets representing the inputs to the specified gates.
         */
        std::vector<Net*> get_input_nets(const std::vector<Gate*>& gates);

        /**
         * @brief Get output nets from a list of gates.
         *
         * This function retrieves the output nets connected to a specified list of gates.
         * By default, only nets with external destinations are considered.
         *
         * @param[in] gates - A vector of gates to analyze.
         * @param[in] only_external_destinations - Boolean flag indicating whether to consider only external destinations (default is true).
         * @returns A vector of nets representing the outputs from the specified gates.
         */
        std::vector<Net*> get_output_nets(const std::vector<Gate*>& gates, bool only_external_destinations = true);
    }    // namespace module_identification
}    // namespace hal
