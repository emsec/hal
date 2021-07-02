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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "z3++.h"

#include <vector>

namespace hal
{
    namespace z3_utils
    {
        struct SubgraphFunctionGenerator
        {
            void get_subgraph_z3_function(const Net* output_net, const std::vector<Gate*> subgraph_gates, z3::context& ctx, z3::expr& result, std::unordered_set<u32>& input_net_ids);

            SubgraphFunctionGenerator() = default;

        private:
            std::map<std::tuple<u32, std::string>, BooleanFunction> m_cache;
            BooleanFunction get_function_of_gate(const Gate* gate, const std::string& out_pin);
        };

        struct RecursiveSubgraphFunctionGenerator
        {
            void get_subgraph_z3_function_recursive(const Net* net, z3::expr& result);

            RecursiveSubgraphFunctionGenerator(z3::context& ctx, const std::vector<Gate*>& subgraph_gates);

        private:
            BooleanFunction get_function_of_gate(const Gate* gate, const std::string& out_pin);
            z3::expr get_function_of_net(const Net* net, z3::context& ctx, const std::vector<Gate*>& subgraph_gates);

            z3::context* m_ctx;
            const std::vector<Gate*> m_subgraph_gates;

            std::map<std::tuple<u32, std::string>, BooleanFunction> m_cache;
            std::map<const Net*, z3::expr> m_expr_cache;
        };

    }    // namespace z3_utils
}    // namespace hal