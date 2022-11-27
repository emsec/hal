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

#include "hal_core/plugin_system/plugin_interface_base.h"

#include "z3++.h"

#include "z3_wrapper.h"

namespace hal
{
    namespace z3_utils
    {
        /**
         * Translates a hal Boolean function into an equivalent z3 expression in the given context. 
         * Replacement expressions for variables can be specified.
         * 
         * @param[in] bf - The Boolean function to translate.
         * @param[in] ctx - The context where the new expression is created in.
         * @param[in] var2expr - Optional replacements for variables.
         * @returns A z3 expression equivalent to the Boolean function.
         */
        z3::expr to_z3(const BooleanFunction& bf, z3::context& context, const std::map<std::string, z3::expr>& var2expr={});

        /**
         * Translates a z3 expression into an equivalent hal Boolean function.
         * 
         * @param[in] e - The expression to translate.
         * @returns A Boolean function equivalent to the  z3 expression.
         */
        BooleanFunction to_hal(const z3::expr& e);

        /**
         * Translates a z3 expression into an equivalent smt2 representation. 
         * 
         * @param[in] e - The expression to translate.
         * @returns A string containing the smt2 representation.
         */
        std::string to_smt2(const z3::expr& e);

        /**
         * Translates a z3 expression into a c++ representation that can be used to evalute the function fast and track the influence of the variables. 
         * 
         * @param[in] e - The expression to translate.
         * @returns A string containing the c++ representation.
         */
        std::string to_cpp(const z3::expr& e);

        /**
         * Translates a z3 expression into a verilog network representation.
         * 
         * @param[in] e - The expression to translate.
         * @param[in] control_mapping - A control mapping that can be applied.
         * @returns A string containing the verilog representation.
         */
        std::string to_verilog(const z3::expr& e, const std::map<std::string, bool>& control_mapping);
        
        /**
         * Extracts all variable names from a z3 expression.
         * 
         * @param[in] e - The expression to extract the variable names from.
         * @returns A set containing all the variable names
         */
        std::set<std::string> get_variable_names(const z3::expr& e);

        /**
         * Extracts all net ids from the variables of a z3 expression.
         * 
         * @param[in] e - The expression to extract the net ids from.
         * @returns A set containing all the net ids.
         */
        std::set<u32> extract_net_ids(const z3::expr& e);

        /**
         * Extracts all net ids from a set of variables.
         * 
         * @param[in] variable_names - The set of variable names.
         * @returns A set containing all the net ids.
         */
        std::set<u32> extract_net_ids(const std::set<std::string>& variable_names);

        /**
         * Get the z3 expression representation of a combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The the z3 expression representation of combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, z3::context& ctx);

        /**
         * Get the z3 expression representations of combined Boolean functions of a subgraph of combinational gates starting at the sources of the provided subgraph output nets.
         * The variables of the resulting Boolean functions are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_outputs - The subgraph oputput nets for which to generate the Boolean functions.
         * @return The the z3 expression representations of combined Boolean functions of the subgraph on success, an error otherwise.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs, z3::context& ctx);

        /**
         * Translates the expr to another context.
         * 
         * @param[in] e - The expression to be translated.
         * @param[in] ctx - Context that the expression is translated into.
         * @returns A copy of the expression inside the new context.
         */
        z3::expr get_expr_in_ctx(const z3::expr& e, z3::context& ctx);

        /**
         * Compare two nets from two different netlist. This is done on a functional level and 
         * compares the combinational gates infront of the nets until hitting a sequential gate.
         * In order for this two work the sequential gates of both netlists must be identical and only the combinational gates my differ.
         * If replace_net_ids is set the function subtitutes the input nets of the functions with their source gate and pin.
         * This allows to also change the output nets of the sequential gates.
         * 
         * @param[in] netlist_a - The first netlist.
         * @param[in] netlist_b - The second netlist.
         * @param[in] net_a - First net, from netlist_a.
         * @param[in] net_b - Second net, from netlist_b.
         * @param[in] replace_net_ids - If set, the input_net_ids are substituted by their source gate and pin.
         * 
         */
        bool compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b,  bool replace_net_ids=true);

    }   // namespace z3_utils


    class PLUGIN_API Z3UtilsPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        BooleanFunction get_subgraph_function_py(const Net* n, const std::vector<Gate*>& sub_graph_gates) const;
    };
}    // namespace hal
