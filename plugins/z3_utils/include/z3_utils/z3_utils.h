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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"
#include "z3++.h"

#include <map>
#include <set>

namespace hal
{
    namespace z3_utils
    {
        /**
         * @brief Translates a hal Boolean function into an equivalent z3 expression in the given context. 
         * 
         * Replacement expressions for variables can be specified.
         * 
         * @param[in] bf - The Boolean function to translate.
         * @param[in] ctx - The context where the new expression is created in.
         * @param[in] var2expr - Optional replacements for variables.
         * @returns A z3 expression equivalent to the Boolean function.
         */
        z3::expr from_bf(const BooleanFunction& bf, z3::context& context, const std::map<std::string, z3::expr>& var2expr = {});

        /**
         * @brief Creates a z3 expression from a binary representation of a value of arbitrary size.
         * 
         * @param[in] ctx - The context where the new expression is created in.
         * @param[in] bit_string - The binary string 
         * @returns A z3 expression equivalent to the Boolean function.
         */
        Result<z3::expr> value_from_binary_string(z3::context& context, const std::string& bit_string);

        /**
         * @brief Translates a z3 expression into an equivalent hal Boolean function.
         * 
         * @param[in] e - The expression to translate.
         * @returns A Boolean function equivalent to the  z3 expression.
         */
        Result<BooleanFunction> to_bf(const z3::expr& e);

        /**
         * @brief Translates a z3 expression into an equivalent smt2 representation. 
         * 
         * @param[in] e - The expression to translate.
         * @returns A string containing the smt2 representation.
         */
        std::string to_smt2(const z3::expr& e);

        /**
         * @brief Translates a z3 expression into a c++ representation that can be used to evalute the function fast and track the influence of the variables. 
         * 
         * @param[in] e - The expression to translate.
         * @returns A string containing the c++ representation.
         */
        std::string to_cpp(const z3::expr& e);

        /**
         * @brief Translates a z3 expression into a verilog network representation.
         * 
         * @param[in] e - The expression to translate.
         * @param[in] control_mapping - A control mapping that can be applied.
         * @returns A string containing the verilog representation.
         */
        std::string to_verilog(const z3::expr& e, const std::map<std::string, bool>& control_mapping = {});

        /**
         * @brief Extracts all variable names from a z3 expression.
         * 
         * @param[in] e - The expression to extract the variable names from.
         * @returns A set containing all the variable names
         */
        std::set<std::string> get_variable_names(const z3::expr& e);

        /**
         * @brief Extracts all net ids from the variables of a z3 expression.
         * 
         * @param[in] e - The expression to extract the net ids from.
         * @returns A set containing all the net ids.
         */
        std::set<u32> extract_net_ids(const z3::expr& e);

        /**
         * @brief Extracts all net ids from a set of variables.
         * 
         * @param[in] variable_names - The set of variable names.
         * @returns A set containing all the net ids.
         */
        std::set<u32> extract_net_ids(const std::set<std::string>& variable_names);

        /**
         * @brief Translates the expr to another context.
         * 
         * @param[in] e - The expression to be translated.
         * @param[in] ctx - Context that the expression is translated into.
         * @returns A copy of the expression inside the new context.
         */
        z3::expr get_expr_in_ctx(const z3::expr& e, z3::context& ctx);

    }    // namespace z3_utils
}    // namespace hal
