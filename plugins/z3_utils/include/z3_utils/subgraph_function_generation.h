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

#include "hal_core/utilities/result.h"
#include "z3++.h"

#include <map>
#include <vector>

namespace hal
{
    class BooleanFunction;
    class Gate;
    class GatePin;
    class Net;

    namespace z3_utils
    {
        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * 
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The the z3 expression representation of combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, z3::context& ctx);


        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * 
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The the z3 expression representation of combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, z3::context& ctx,  std::map<u32, z3::expr>& net_cache,
            std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief  Get the z3 expression representations of combined Boolean functions of a subgraph of combinational gates starting at the sources of the provided subgraph output nets.
         * 
         * The variables of the resulting Boolean functions are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_outputs - The subgraph oputput nets for which to generate the Boolean functions.
         * @return The the z3 expression representations of combined Boolean functions of the subgraph on success, an error otherwise.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs, z3::context& ctx);
    }    // namespace z3_utils
}    // namespace hal
