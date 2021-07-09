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

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;

    namespace z3_utils
    {
        class z3Wrapper
        {
        public:
            /**
             * Constructor that generates a Z3Wrapper around a z3::expr and provides usefull utility tools for said expr.
             * This class was constructed with multithreading in mind and therefore you need to pass the context in which the expr existst as 
             * an unique pointer. This is necessary, since multiple threads cannot work on the same context and a context is not copyable.
             * If you need to work with different expr in different wrappers in the same context you need to translate the expr into the same context first.
             *
             * @param[in] ctx - Unique Pointer to the context of the expr that is being wrapped.
             * @param[in] expr - Expression to wrap.
             * @returns The Z3Wrapper around the provided expression.
             */
            z3Wrapper(std::unique_ptr<z3::context> ctx, std::unique_ptr<z3::expr> expr);


            /**
             * Specifc Function used to optimize z3 epxr with the help of the logic synthesis tool ABC - CURRENTLY NOT IMPLEMENTED
             */
            // bool optimize();

            /**
             * We evaluate the boolean function represented by the expression a relevant number of times twice for each input to the function.
             * One time we set the input under inspection to 0 and the other time to 1.
             * We check wether the overall output changed and record the total amount of changes.
             * We define the boolean influence to be estimated by the ratio between the total amount of changes to the total amount if executions.
             * 
             * @param[in] num_evaluations - Number of random evaluations that are used for the estimation.
             * 
             * @returns A mapping of the net_ids in the wrapped expr to their estimated boolean influence.
             */
            std::unordered_map<u32, double> get_boolean_influence(const u32 num_evaluations=32000) const;

            /**
             * Generates smt2 representation of the wrapped expression.
             * 
             * @returns The smt2 representation of the wrapped expression as string.
             */
            std::string get_smt2_string() const;

            /**
             * Every Z3Wrapper gets assigned an id that can be used to distinguish them in case of multithreading.
             * 
             * @returns The id of the  Z3Wrapper.
             */
            u32 get_id() const;

            /**
             * Getter to the wrapped expr.
             * 
             * @returns A copy of the wrapped expr.
             */
            z3::expr get_expr() const;

            /**
             * Translates the wrapped expr to another context.
             * 
             * @param[in] ctx - Context that the wrapped expression is translated into.
             * @returns A copy of the wrapped expression inside the given context.
             */
            z3::expr get_expr_in_ctx(z3::context& ctx) const;

            /**
             * Getter to the internal data structure that maps the hal net_ids in the wrapped expression to z3 expressions representing the net_ids.
             * 
             * @returns A copy of the internal data structure that maps the hal net_ids in the wrapped expression to z3 expressions representing the net_ids.
             */
            std::unordered_map<u32, z3::expr> get_input_mapping_hal_to_z3() const;

            /**
             * Getter to the internal data structure that includes the expressions representing the net_ids in the wrapped expression.
             * 
             * @returns A copy of the internal data structure that includes the expressions representing the net_ids in the wrapped expression.
             */
            std::vector<z3::expr> get_inputs_z3_expr() const;

            /**
             * Getter to the internal data structure that includes the net_ids in the wrapped expression.
             * 
             * @returns A copy of the internal data structure that includes the net_ids in the wrapped expression.
             */
            std::vector<u32> get_inputs_net_ids() const;

            /**
             * Transforms the wrapped z3 expression into verligo syntax and writes it to a file.
             * 
             * @param[in] path - Path where the file is written to.
             * @returns True in case of success, false otherwise.
             */
            bool write_verilog_file(const std::filesystem::path& path, const std::map<u32, bool>& control_mapping={}) const;

            /**
             * Transforms the wrapped z3 expression into efficent c code and writes it to a file.
             * 
             * @param[in] path - Path where the file is written to.
             * @returns True in case of success, false otherwise.
             */
            bool write_c_file(const std::filesystem::path& path) const;

            /**
             * Removes all net ids from the wrapped expression that are driven by GND or VCC gates and replaces them with constant 0/1 expressions.
             * 
             * @param[in] nl - Pointer to the netlist that includedes the GND and VCC gates to replace.
             */
            void remove_global_inputs(const Netlist* nl);

            /**
             * Removes all static constants from the wrapped expression and replaces them with nets that are driven by GND or VCC gates.
             * 
             * @param[in] nl - Pointer to the netlist that includedes the GND and VCC gates to replace.
             */
            void remove_static_inputs(const Netlist* nl);

        private:
            u32 m_z3_wrapper_id;
            static u32 create_id();

            bool extract_function_inputs();

            std::unique_ptr<z3::context> m_ctx;
            std::unique_ptr<z3::expr> m_expr;

            std::unordered_map<u32, z3::expr> m_input_mapping_hal_to_z3;
            std::vector<z3::expr> m_inputs_z3_expr;
            std::vector<u32> m_inputs_net_ids;
        };
    }    // namespace z3_utils
}    // namespace hal