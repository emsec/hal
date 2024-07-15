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

#pragma once

#include "hal_core/utilities/result.h"
#include "z3++.h"

#include <unordered_map>

namespace hal
{
    namespace z3_utils
    {
        /**
         * @brief Applies hand-crafted simplification rules iteratively until no further simplifications can be made.
         * 
         * @param[in] e The Z3 expression to be simplified.
         * @param[in] cache A cache to store simplified (sub)expressions.
         * @param[in] check_correctness A flag to check the correctness of each simplification step. Default is false.
         * @returns OK() and the simplified Z3 expression in case of success, an error otherwise.
         */
        Result<z3::expr> simplify_local(const z3::expr& e, std::unordered_map<u32, z3::expr>& cache, const bool check_correctness = false);

        /**
         * @brief Applies hand-crafted simplification rules iteratively until no further simplifications can be made.
         * 
         * @param[in] e The Z3 expression to be simplified.
         * @param[in] check_correctness A flag to check the correctness of each simplification step. Default is false.
         * @returns OK() and the simplified Z3 expression in case of success, an error otherwise.
         */
        Result<z3::expr> simplify_local(const z3::expr& e, const bool check_correctness = false);

    }    // namespace z3_utils
}    // namespace hal
