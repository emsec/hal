//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "z3++.h"
#include "z3_utils/include/z3Wrapper.h"

#include <map>
#include <vector>

namespace hal
{
    namespace z3_utils
    {
        class Converter
        {
        public:
            enum Operation
            {
                bvand,
                bvnot,
                bvor,
                bvxor,
                NONE,
            };

            /**
                 * This base function prases and splits up the smt representation of a wrapped z3 expression and converts it into another representation 
                 * depending on the child class that overrides the virtual methods. 
                 *
                 * @param[in] e - Z3Wrapper wrapping the expression that is converted.
                 * @returns The converted expression as a string.
                 */
            std::string convert_z3_expr_to_func(const z3Wrapper& e) const;

        protected:
            // VIRTUAL METHODS
            virtual std::string build_operand(const std::string& operand) const                                                                            = 0;
            virtual std::string build_operation(const Operation& operation, const std::vector<std::string>& operands) const                                = 0;
            virtual std::string generate_assignment(const std::string& l) const                                                                            = 0;
            virtual std::string generate_initialization(const std::vector<u32>& inputs) const                                                              = 0;
            virtual std::string construct_function(const std::string& assignments, const std::string& initalization, const std::vector<u32>& inputs) const = 0;

            // UTILS
            std::string integer_with_leading_zeros(const u32 i, const u32 total_length) const;
            std::string replace_all(const std::string& str, const std::string& from, const std::string& to) const;

            std::string operation_to_string(const Operation& op) const;

            // GENERAL METHODS
            bool does_line_contain_assignment(const std::string& l) const;
            std::string extract_lhs(const std::string& l) const;

            std::map<u32, u32> extract_paranthesis_pairs(const std::string& l) const;
            bool contains_one_operation(const std::string& l) const;
            std::map<std::string, std::string> extract_sub_exrepssions(const std::string& l) const;

            Operation extract_operation(const std::string& se) const;
            std::vector<std::string> extract_operands(const std::string& se) const;
            std::map<std::string, std::string> translate_sub_expressions(const std::map<std::string, std::string>& se) const;

            std::string merge_sub_expressions(const std::map<std::string, std::string>& translated_sub_expressions) const;

            std::string generate_function(const std::vector<std::string>& assignments, std::vector<u32>& inputs) const;

            // supported operations
            std::vector<Operation> m_operations = {Converter::bvand, Converter::bvnot, Converter::bvor, Converter::bvxor};
        };
    }    //namespace z3_utils
}    // namespace hal