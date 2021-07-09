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

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

namespace hal
{
    /* forward declaration */
    class Gate;
    class Netlist;

    namespace dataflow
    {
        struct Grouping;

        struct NetlistAbstraction
        {
            NetlistAbstraction(Netlist* nl_arg);

            std::shared_ptr<Grouping> create_initial_grouping(std::set<std::set<u32>> known_groups) const;

            // netlist
            Netlist* nl;

            // utils
            bool yosys;

            // all ffs
            std::vector<Gate*> all_sequential_gates;

            /* pre_processed_data */
            std::unordered_map<u32, std::vector<u32>> gate_to_fingerprint;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_clock_signals;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_enable_signals;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_reset_signals;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_set_signals;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_register_stages;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_predecessors;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_successors;
            std::unordered_map<u32, std::vector<std::vector<u32>>> gate_to_output_shape;
            std::unordered_map<u32, std::vector<std::vector<u32>>> gate_to_input_shape;
        };
    }    // namespace dataflow
}    // namespace hal