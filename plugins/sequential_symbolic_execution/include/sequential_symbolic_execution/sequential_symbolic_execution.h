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
 * @file sequential_symbolic_execution.h
 * @brief This file contains the declarations for functions to perform sequential symbolic execution using z3.
 */

#pragma once

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/result.h"
#include "z3_utils/z3_utils.h"

#include <map>
#include <vector>

namespace hal
{
    class Gate;
    class Module;
    class Net;
    class Netlist;

    namespace sse
    {
        /**
         * @brief Get the Z3 expression representing the value of a net at a specific time index.
         *
         * This function computes the Z3 expression for the value of a given net at a specific time index.
         * It considers the known input values, subgraph gates, and word-level calculations.
         *
         * @param[in] net - The target net.
         * @param[in] time_index - The time index to evaluate.
         * @param[in] subgraph_gates_byte_map - A byte map indicating the included gates in the subgraph.
         * @param[in] known_inputs - A vector of maps containing known input values for each time index.
         * @param[in] word_level_calculations - A map of pin groups to their corresponding Boolean functions.
         * @param[in] substitute_endpoints - Whether to substitute endpoints with pins of identified modules.
         * @param[in] ctx - The Z3 context.
         * @returns OK() and the Z3 expression on success, an error otherwise.
         */
        Result<z3::expr> get_value_at_z3(Net* net,
                                         const u32 time_index,
                                         const std::vector<bool>& subgraph_gates_byte_map,
                                         const std::vector<std::map<u32, bool>>& known_inputs,
                                         const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                         const bool substitute_endpoints,
                                         z3::context& ctx);

        /**
         * @brief Get the Z3 expressions representing the values of multiple nets as words at specific time indices.
         *
         * This function computes the Z3 expressions for the values of multiple nets at specific time indices, treating them as words.
         * It considers the known input values, subgraph gates, and word-level calculations.
         *
         * @param[in] words - A vector of vectors of nets representing words.
         * @param[in] time_indices - A vector of time indices for each word.
         * @param[in] subgraph_gates_byte_map - A byte map indicating the included gates in the subgraph.
         * @param[in] known_inputs - A vector of maps containing known input values for each time index.
         * @param[in] word_level_calculations - A map of pin groups to their corresponding Boolean functions.
         * @param[in] substitute_endpoints - Whether to substitute endpoints with pins of identified modules.
         * @param[in] ctx - The Z3 context.
         * @returns OK() and a vector of Z3 expressions on success, an error otherwise.
         */
        Result<std::vector<z3::expr>> get_word_values_at_z3(const std::vector<std::vector<Net*>>& words,
                                                            const std::vector<u32>& time_indices,
                                                            const std::vector<bool>& subgraph_gates_byte_map,
                                                            const std::vector<std::map<u32, bool>>& known_inputs,
                                                            const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                            const bool substitute_endpoints,
                                                            z3::context& ctx);

        /**
         * @brief Get the Z3 expressions representing the values of pin groups as words at specific time indices.
         *
         * This function computes the Z3 expressions for the values of pin groups at specific time indices, treating them as words.
         * It considers the known input values, subgraph gates, and word-level calculations.
         *
         * @param[in] words - A vector of pairs of modules and pin groups representing words.
         * @param[in] time_indices - A vector of time indices for each word.
         * @param[in] subgraph_gates_byte_map - A byte map indicating the included gates in the subgraph.
         * @param[in] known_inputs - A vector of maps containing known input values for each time index.
         * @param[in] word_level_calculations - A map of pin groups to their corresponding Boolean functions.
         * @param[in] substitute_endpoints - Whether to substitute endpoints with pins of identified modules.
         * @param[in] ctx - The Z3 context.
         * @returns OK() and a vector of Z3 expressions on success, an error otherwise.
         */
        Result<std::vector<z3::expr>> get_word_values_at_z3(const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& words,
                                                            const std::vector<u32>& time_indices,
                                                            const std::vector<bool>& subgraph_gates_byte_map,
                                                            const std::vector<std::map<u32, bool>>& known_inputs,
                                                            const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                                                            const bool substitute_endpoints,
                                                            z3::context& ctx);

    }    // namespace sse
}    // namespace hal
