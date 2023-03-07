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
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class NETLIST_API SubgraphNetlistDecorator
    {
    public:
        /**
         * Construct new SubgraphNetlistDecorator object.
         * 
         * @param[in] netlist - The netlist to operate on.
         */
        SubgraphNetlistDecorator(const Netlist& netlist);

        /**
         * Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph that shall be copied from the netlist.
         * @return The copied subgraph netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const std::vector<const Gate*>& subgraph_gates) const;

        /**
         * Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph that shall be copied from the netlist.
         * @return The copied subgraph netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const std::vector<Gate*>& subgraph_gates) const;

        /**
         * Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.
         * 
         * @param[in] subgraph_module - The module making up the subgraph that shall be copied from the netlist.
         * @return The copied subgraph netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const Module* subgraph_module) const;

        /**
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * Utilizes a cache for speedup on consecutive calls.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction>
            get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const;

        /**
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * Utilizes a cache for speedup on consecutive calls.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const;

        /**
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * Utilizes a cache for speedup on consecutive calls.
         * 
         * @param[in] subgraph_module - The module making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @param[inout] cache - Cache to speed up computations. The cache is filled by this function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const;

        /**
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_module - The module making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * This does not actually build the boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * 
         * @param[in] subgraph_gates - The subgraph gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net from which to start the back propagation from.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise;
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * This does not actually build the boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * 
         * @param[in] subgraph_gates - The subgraph gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net from which to start the back propagation from.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise;
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * This does not actually build the boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * 
         * @param[in] subgraph_module - The module making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net from which to start the back propagation from.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise;
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const Module* subgraph_module, const Net* subgraph_output) const;

    private:
        const Netlist& m_netlist;
    };
}    // namespace hal