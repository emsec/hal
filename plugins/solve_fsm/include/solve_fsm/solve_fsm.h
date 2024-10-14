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
 * @file solve_fsm.h 
 * @brief This file contains functions to generate the state transition graph of a given FSM.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/result.h"

#include <map>

namespace hal
{
    class Netlist;
    class Gate;

    namespace solve_fsm
    {
        /**
         * Generate the state transition graph of a given FSM using SMT solving.
         * The result is a map from each state of the FSM to all of its transitions.
         * A transition is given as each successor state as well as the Boolean condition that needs to be fulfilled for the transition to take place.
         * Optionally also produces a DOT file representing the state transition graph.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] state_reg - A vector of flip-flop gates that make up the state register of the FSM.
         * @param[in] transition_logic - A vector of combinational gates that make up the transition logic of the FSM.
         * @param[in] initial_state - A map from the state register flip-flops to their initial (Boolean) value. If an empty map is provided, the initial state is set to 0. Defaults to an empty map.
         * @param[in] graph_path - File path at which to store the DOT state transition graph. No file is created if the path is left empty. Defaults to an empty path.
         * @param[in] timeout - Timeout for the underlying SAT solvers. Defaults to 600000 ms.
         * @returns OK() and a map from each state to its successor states as well as the condition for the respective transition to be taken, an error otherwise.
         */
        Result<std::map<u64, std::map<u64, BooleanFunction>>> solve_fsm(Netlist* nl,
                                                                        const std::vector<Gate*>& state_reg,
                                                                        const std::vector<Gate*>& transition_logic,
                                                                        const std::map<Gate*, bool>& initial_state = {},
                                                                        const std::filesystem::path& graph_path    = "",
                                                                        const u32 timeout                          = 600000);

        /**
         * Generate the state transition graph of a given FSM using brute force.
         * The result is a map from each state of the FSM to all of its transitions.
         * A transition is given as each successor state as well as the Boolean condition that needs to be fulfilled for the transition to take place.
         * Optionally also produces a DOT file representing the state transition graph.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] state_reg - A vector of flip-flop gates that make up the state register of the FSM.
         * @param[in] transition_logic - A vector of combinational gates that make up the transition logic of the FSM.
         * @param[in] graph_path - File path at which to store the DOT state transition graph. No file is created if the path is left empty. Defaults to an empty path.
         */
        Result<std::map<u64, std::map<u64, BooleanFunction>>>
            solve_fsm_brute_force(Netlist* nl, const std::vector<Gate*>& state_reg, const std::vector<Gate*>& transition_logic, const std::filesystem::path& graph_path = "");

        /**
         * Generates the state graph of a finite state machine from the transitions of that fsm.
         *
         * @param[in] state_reg - Vector contianing the state registers.
         * @param[in] transitions - Transitions of the fsm given as a map from origin state to all possible successor states and the corresponding condition.
         * @param[in] graph_path - Path where the transition state graph in dot format is saved.
         * @param[in] max_condition_length - The maximum character length that is printed for boolean functions representing the conditions.
         * @param[in] base - The base with that the states are formatted and printed.
         * @returns A string representing the dot graph.
         */
        Result<std::string> generate_dot_graph(const std::vector<Gate*>& state_reg,
                                               const std::map<u64, std::map<u64, BooleanFunction>>& transitions,
                                               const std::filesystem::path& graph_path = "",
                                               const u32 max_condition_length          = 128,
                                               const u32 base                          = 10);
    }    // namespace solve_fsm
}    // namespace hal