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
 * @file configuration.h
 * @brief This file contains the struct that holds the configuration of a run of the FSM solver.
 */

#pragma once

#include "hal_core/defines.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace hal
{
    class Gate;
    class Net;
    class Netlist;

    namespace solve_fsm
    {
        /**
         * @struct Configuration
         * @brief The configuration of a run of the FSM solver.
         *
         * Holds everything the solver needs to know about the FSM, including the netlist that implements it. The state
         * register and the transition logic are mandatory, everything else is optional.
         *
         * States are encoded as integers, with the first flip-flop of the state register providing the least
         * significant bit.
         */
        struct Configuration
        {
            /**
             * @brief Construct a new FSM solver configuration for the given netlist.
             *
             * @param[in] nl - The netlist that implements the FSM.
             */
            Configuration(Netlist* nl);

            /**
             * @brief The netlist that implements the FSM.
             */
            Netlist* netlist;

            /**
             * @brief The flip-flops that make up the state register of the FSM.
             *
             * The first flip-flop provides the least significant bit of the state. Defaults to an empty vector, but a
             * state register is required for the solver to run.
             */
            std::vector<Gate*> state_register = {};

            /**
             * @brief The combinational gates that compute the next state of the FSM.
             *
             * Defaults to an empty vector, but transition logic is required for the solver to run.
             */
            std::vector<Gate*> transition_logic = {};

            /**
             * @brief The outputs of the FSM, each given as a name and the nets that make up that output.
             *
             * The first net of an output provides its least significant bit, so a single-bit output is a vector
             * holding one net. Defaults to an empty vector, in which case no outputs are computed.
             */
            std::vector<std::pair<std::string, std::vector<Net*>>> outputs = {};

            /**
             * @brief The initial value of each flip-flop of the state register.
             *
             * Only states reachable from the resulting initial state are explored. Defaults to an empty map, in which
             * case the FSM starts in state 0.
             */
            std::map<Gate*, bool> initial_state = {};

            /**
             * @brief The timeout for the underlying SMT solver in milliseconds. Defaults to 600000 ms.
             *
             * Has no effect when `brute_force` is set, as no SMT solver is used then.
             */
            u32 timeout = 600000;

            /**
             * @brief Enumerate all states instead of using an SMT solver. Defaults to `false`.
             *
             * Brute forcing needs no external solver and is faster for small state registers, but its runtime doubles
             * with every additional flip-flop. Both approaches produce the same state transition graph.
             */
            bool brute_force = false;

            /**
             * @brief Set the flip-flops that make up the state register of the FSM.
             *
             * @param[in] state_register - The flip-flops of the state register, least significant bit first.
             * @returns The updated FSM solver configuration.
             */
            Configuration& with_state_register(const std::vector<Gate*>& state_register);

            /**
             * @brief Set the combinational gates that compute the next state of the FSM.
             *
             * @param[in] transition_logic - The gates of the transition logic.
             * @returns The updated FSM solver configuration.
             */
            Configuration& with_transition_logic(const std::vector<Gate*>& transition_logic);

            /**
             * @brief Set the outputs of the FSM that the solver should evaluate in each state.
             *
             * @param[in] outputs - The outputs, each given as a name and the nets that make up that output, least significant bit first.
             * @returns The updated FSM solver configuration.
             */
            Configuration& with_outputs(const std::vector<std::pair<std::string, std::vector<Net*>>>& outputs);

            /**
             * @brief Set the initial value of each flip-flop of the state register.
             *
             * @param[in] initial_state - The initial value of each flip-flop of the state register.
             * @returns The updated FSM solver configuration.
             */
            Configuration& with_initial_state(const std::map<Gate*, bool>& initial_state);

            /**
             * @brief Set the timeout for the underlying SMT solver.
             *
             * @param[in] timeout - The timeout in milliseconds.
             * @returns The updated FSM solver configuration.
             */
            Configuration& with_timeout(const u32 timeout);

            /**
             * @brief Set whether to enumerate all states instead of using an SMT solver.
             *
             * @param[in] brute_force - Set `true` to enumerate all states, `false` to use an SMT solver. Defaults to `true`.
             * @returns The updated FSM solver configuration.
             */
            Configuration& with_brute_force(const bool brute_force = true);
        };
    }    // namespace solve_fsm
}    // namespace hal
