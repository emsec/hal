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
 * @brief This file contains the function to recover the state transition graph of a finite state machine.
 */

#pragma once

#include "hal_core/utilities/result.h"
#include "solve_fsm/configuration.h"
#include "solve_fsm/state_transition_graph.h"

namespace hal
{
    /**
     * @brief Recovers the state transition graph of a finite state machine from the gate-level netlist that implements it.
     */
    namespace solve_fsm
    {
        /**
         * @brief Recover the state transition graph of an FSM from the netlist that implements it.
         *
         * Explores the states that are reachable from the initial state and determines, for each of them, which
         * successor states it can reach and under which condition. If outputs are configured, the value of each output
         * in each state is computed as well.
         *
         * No file is written. Use `StateTransitionGraph::generate_dot_graph` on the result to render the graph.
         *
         * @param[in] config - The configuration of the FSM solver run.
         * @returns OK() and the state transition graph of the FSM on success, an error otherwise.
         */
        Result<StateTransitionGraph> solve_fsm(const Configuration& config);
    }    // namespace solve_fsm
}    // namespace hal
