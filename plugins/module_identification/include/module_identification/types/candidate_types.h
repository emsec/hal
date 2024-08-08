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
 * @file candidate_types.h 
 * @brief This file contains the enumeration and constants for the candidate types used in the module identification process.
 */

#pragma once

#include <map>
#include <vector>

namespace hal
{
    namespace module_identification
    {
        /**
         * @enum CandidateType
         * @brief Enumeration of the different candidate types for module identification.
         * 
         * This enum specifies the types of operations that the module identification process can recognize and verify, such as arithmetic operations and comparisons.
         */
        enum class CandidateType
        {
            /** @brief Addition operation. */
            adder = 0,

            /** @brief Subtraction operation. */
            subtraction,

            /** @brief Counter operation. */
            counter,

            /** @brief Negation operation. */
            negation,

            /** @brief Absolute value operation. */
            absolute,

            /** @brief Constant multiplication operation. */
            constant_multiplication,

            /** @brief Equality comparison. */
            equal,

            /** @brief Less-than comparison. */
            less_than,

            /** @brief Less-than-or-equal comparison. */
            less_equal,

            /** @brief Signed less-than comparison. */
            signed_less_than,

            /** @brief Signed less-than-or-equal comparison. */
            signed_less_equal,

            /** @brief Value check against a constant operation. */
            value_check,

            /** @brief No operation. */
            none,

            /** @brief´Mixed operation, for merged `VerifiedCandidates` that contain multiple candidate types. */
            mixed,
        };

        /**
         * @brief A list of all candidate types that are selectable to be checked.
         * 
         * This list does not contain all of the types that we can verify since some of the types are verified in the same way, like addition and subtraction, so selecting one implicitly selects the other.
         * This selection is not meant to filter the types you want to verify for quality of results (for example if you want to prevent an addition that you consider would be more suitable identified as a subtrataction).
         * The purpose of disableing some types for checking is for perfomance reasons.
         */
        const std::vector<CandidateType> all_checkable_candidate_types = {
            CandidateType::adder,
            CandidateType::counter,
            CandidateType::absolute,

            CandidateType::constant_multiplication,

            CandidateType::equal,
            CandidateType::less_equal,

            CandidateType::value_check,
        };

        /**
         * @brief A mapping of candidate types to their maximum number of operands.
         * 
         * This map provides the maximum number of operands for each candidate type that the module identification process can handle.
         */
        const std::map<CandidateType, u32> candidate_type_max_operands = {
            {CandidateType::counter, 1},
            {CandidateType::absolute, 1},
            {CandidateType::constant_multiplication, 1},

            {CandidateType::equal, 2},
            {CandidateType::less_equal, 2},

            {CandidateType::adder, 3},
        };
    }    // namespace module_identification
}    // namespace hal
