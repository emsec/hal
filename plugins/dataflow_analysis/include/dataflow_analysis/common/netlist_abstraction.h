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

/**
 * @file netlist_abstraction.h
 * @brief This file contains the struct that holds all information on the netlist abstraction used for dataflow analysis.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Gate;
    class Netlist;

    namespace dataflow
    {
        struct Grouping;

        /**
         * @struct NetlistAbstraction
         * @brief The abstraction of the netlist that only contains gates of a specified type, e.g., flip-flops.
         */
        struct NetlistAbstraction
        {
            /**
             * @brief Construct a netlist abstraction from a netlist.
             * 
             * @param[in] nl_arg - The netlist.
             */
            NetlistAbstraction(const Netlist* nl_arg);

            /**
             * The netlist associated with the netlist abstraction.
             */
            const Netlist* nl;

            // utils
            bool yosys;

            /**
             * The target gates that should be grouped by dataflow analysis.
             */
            std::vector<Gate*> target_gates;

            /* pre_processed_data */
            std::unordered_map<u32, std::vector<u32>> gate_to_fingerprint;
            std::unordered_map<u32, std::map<PinType, std::unordered_set<u32>>> gate_to_control_signals;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_register_stages;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_predecessors;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_successors;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_known_predecessor_groups;
            std::unordered_map<u32, std::unordered_set<u32>> gate_to_known_successor_groups;
            std::unordered_map<u32, std::vector<std::vector<u32>>> gate_to_output_shape;
            std::unordered_map<u32, std::vector<std::vector<u32>>> gate_to_input_shape;
        };
    }    // namespace dataflow
}    // namespace hal