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

#include "hal_core/defines.h"
#include "z3++.h"

#include <map>
#include <vector>

namespace hal
{
    namespace z3_utils
    {
        /**
         * The base class for the converters that translate a Boolean function given in SMT-LIB notation into another language.
         */
        class Converter
        {
        public:
            /**
             * The bit-vector operations that the converters can translate.
             */
            enum Operation
            {
                bvand, /**< Bitwise AND. */
                bvnot, /**< Bitwise NOT. */
                bvor,  /**< Bitwise OR. */
                bvxor, /**< Bitwise XOR. */
                NONE,  /**< No operation, returned when no known operation could be extracted. */
            };

            /**
             * This base function prases and splits up the smt representation of a wrapped z3 expression and converts it into another representation 
             * depending on the child class that overrides the virtual methods. 
             *
             * @param[in] e - z3 expression that is converted.
             * @returns The converted expression as a string.
             */
            std::string convert_z3_expr_to_func(const z3::expr& e) const;

        protected:
            // VIRTUAL METHODS

            /**
             * Translate a single operand into the target language.
             *
             * @param[in] operand - The operand in SMT-LIB notation.
             * @returns The operand in the target language.
             */
            virtual std::string build_operand(const std::string& operand) const = 0;

            /**
             * Translate a single operation and its operands into the target language.
             *
             * @param[in] operation - The operation to translate.
             * @param[in] operands - The operands of the operation, already translated.
             * @returns The operation in the target language.
             */
            virtual std::string build_operation(const Operation& operation, const std::vector<std::string>& operands) const = 0;

            /**
             * Translate a single assignment of the SMT-LIB representation into the target language.
             *
             * @param[in] l - The line holding the assignment.
             * @returns The assignment in the target language.
             */
            virtual std::string generate_assignment(const std::string& l) const = 0;

            /**
             * Generate the code that reads the input variables before the translated function is evaluated.
             *
             * @param[in] input_vars - The input variables of the function.
             * @returns The initialization code in the target language.
             */
            virtual std::string generate_initialization(const std::vector<std::string>& input_vars) const = 0;

            /**
             * Assemble the complete function of the target language from its parts.
             *
             * @param[in] assignments - The translated assignments that make up the body.
             * @param[in] initalization - The translated initialization code.
             * @param[in] input_vars - The input variables of the function.
             * @returns The complete function in the target language.
             */
            virtual std::string construct_function(const std::string& assignments, const std::string& initalization, const std::vector<std::string>& input_vars) const = 0;

            // UTILS

            /**
             * Convert an integer into a string of the given length, padded with leading zeros.
             *
             * @param[in] i - The integer to convert.
             * @param[in] total_length - The length of the resulting string.
             * @returns The padded string.
             */
            std::string integer_with_leading_zeros(const u32 i, const u32 total_length) const;

            /**
             * Replace every occurrence of a substring within a string.
             *
             * @param[in] str - The string to operate on.
             * @param[in] from - The substring to replace.
             * @param[in] to - The replacement.
             * @returns The resulting string.
             */
            std::string replace_all(const std::string& str, const std::string& from, const std::string& to) const;

            /**
             * Get the SMT-LIB name of an operation.
             *
             * @param[in] op - The operation.
             * @returns The name of the operation.
             */
            std::string operation_to_string(const Operation& op) const;

            // GENERAL METHODS

            /**
             * Check whether the given line of the SMT-LIB representation holds an assignment.
             *
             * @param[in] l - The line to check.
             * @returns `true` if the line holds an assignment, `false` otherwise.
             */
            bool does_line_contain_assignment(const std::string& l) const;

            /**
             * Get the variable that is assigned to in the given line.
             *
             * @param[in] l - The line holding the assignment.
             * @returns The name of the assigned variable.
             */
            std::string extract_lhs(const std::string& l) const;

            /**
             * Get the matching parenthesis for every opening parenthesis of the given line.
             *
             * @param[in] l - The line to analyze.
             * @returns A map from the position of each opening parenthesis to the position of its counterpart.
             */
            std::map<u32, u32> extract_paranthesis_pairs(const std::string& l) const;

            /**
             * Check whether the given line consists of exactly one operation, i.e., whether it needs no further splitting.
             *
             * @param[in] l - The line to check.
             * @returns `true` if the line contains exactly one operation, `false` otherwise.
             */
            bool contains_one_operation(const std::string& l) const;

            /**
             * Split the given line into the sub-expressions that it is composed of.
             *
             * @param[in] l - The line to split.
             * @returns A map from a placeholder to the sub-expression that it stands for.
             */
            std::map<std::string, std::string> extract_sub_exrepssions(const std::string& l) const;

            /**
             * Get the operation that the given sub-expression applies.
             *
             * @param[in] se - The sub-expression.
             * @returns The operation, `Converter::NONE` if no known operation was found.
             */
            Operation extract_operation(const std::string& se) const;

            /**
             * Get the operands that the given sub-expression applies its operation to.
             *
             * @param[in] se - The sub-expression.
             * @returns The operands.
             */
            std::vector<std::string> extract_operands(const std::string& se) const;

            /**
             * Translate every sub-expression into the target language.
             *
             * @param[in] se - A map from a placeholder to the sub-expression that it stands for.
             * @returns A map from each placeholder to the translated sub-expression.
             */
            std::map<std::string, std::string> translate_sub_expressions(const std::map<std::string, std::string>& se) const;

            /**
             * Substitute the translated sub-expressions back into one another to form a single expression.
             *
             * @param[in] translated_sub_expressions - A map from each placeholder to the translated sub-expression.
             * @returns The merged expression.
             */
            std::string merge_sub_expressions(const std::map<std::string, std::string>& translated_sub_expressions) const;

            /**
             * Translate all assignments into the target language and assemble them into a complete function.
             *
             * @param[in] assignments - The assignments of the SMT-LIB representation.
             * @param[inout] input_vars - The input variables of the function, filled by this function.
             * @returns The complete function in the target language.
             */
            std::string generate_function(const std::vector<std::string>& assignments, std::vector<std::string>& input_vars) const;

            /** The operations that the converter supports. */
            std::vector<Operation> m_operations = {Converter::bvand, Converter::bvnot, Converter::bvor, Converter::bvxor};
        };
    }    //namespace z3_utils
}    // namespace hal