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
    class SubgraphNetlistDecorator
    {
    public:
        SubgraphNetlistDecorator(const Netlist& netlist);
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const std::vector<const Gate*>& subgraph_gates) const;
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const std::vector<Gate*>& subgraph_gates) const;
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const Module* subgraph_module) const;
        Result<BooleanFunction>
            get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const;
        Result<BooleanFunction> get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const;
        Result<BooleanFunction> get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache) const;
        Result<BooleanFunction> get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const;
        Result<BooleanFunction> get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const;
        Result<BooleanFunction> get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output) const;

    private:
        const Netlist& m_netlist;
    };
}    // namespace hal