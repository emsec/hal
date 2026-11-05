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
 * @file state_transition_graph.h
 * @brief This file contains the struct that holds the state transition graph of an FSM.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/result.h"

#include <filesystem>
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
         * @struct StateTransitionGraph
         * @brief The state transition graph of an FSM, i.e., the behavior that its netlist implements.
         *
         * States are encoded as integers, with the first flip-flop of the state register providing the least
         * significant bit. The same holds for the nets of a multi-bit output.
         */
        struct StateTransitionGraph
        {
            /**
             * @brief The netlist that implements the FSM.
             */
            Netlist* netlist = nullptr;

            /**
             * @brief The flip-flops that make up the state register, in the order that determines the encoding of a state.
             *
             * The first flip-flop provides the least significant bit, so this is what maps a state back to the netlist.
             */
            std::vector<Gate*> state_register;

            /**
             * @brief The outputs of the FSM, each given as a name and the nets that make up that output.
             *
             * The first net of an output provides its least significant bit. Empty unless outputs were configured.
             */
            std::vector<std::pair<std::string, std::vector<Net*>>> output_nets;

            /**
             * @brief A map from each state to its successor states, together with the condition under which the respective transition is taken.
             */
            std::map<u64, std::map<u64, BooleanFunction>> transitions;

            /**
             * @brief A map from each state to the value of every output of the FSM in that state.
             *
             * The outputs of a state are given in the order in which they were configured. An output of a Moore FSM
             * only depends on the state, so its Boolean function is constant. An output of a Mealy FSM may also depend
             * on the inputs of the FSM, in which case its Boolean function still contains the input variables.
             *
             * Empty unless outputs were configured.
             */
            std::map<u64, std::vector<std::pair<std::string, BooleanFunction>>> outputs;

            /**
             * @brief Get the number of flip-flops that make up the state register, i.e., the bit-size of a state.
             *
             * @returns The bit-size of a state.
             */
            u32 get_state_size() const;

            /**
             * @brief Render the state transition graph in the DOT format.
             *
             * Each state becomes a node labeled with its value and, if outputs were computed, with the value of every
             * output in that state. Each transition becomes an edge labeled with its condition. Boolean functions are
             * truncated to keep the graph readable, use `to_string` to get them in full.
             *
             * @param[in] graph_path - The file path at which to store the graph. No file is written if the path is left empty. Defaults to an empty path.
             * @param[in] max_condition_length - The maximum number of characters printed for a Boolean function. Defaults to 128.
             * @param[in] base - The base in which state and output values are printed, either 2 or 10. Defaults to 10.
             * @returns OK() and the graph in the DOT format on success, an error otherwise.
             */
            Result<std::string> generate_dot_graph(const std::filesystem::path& graph_path = "", const u32 max_condition_length = 128, const u32 base = 10) const;

            /**
             * @brief Render the state transition graph as human-readable text, without truncating anything.
             *
             * Starts with a legend that maps each bit of the state to the flip-flop holding it, each output to the
             * nets that make it up, and every net variable appearing in a Boolean function to the net it stands for.
             * The legend is followed by one block per state holding its outputs and all of its outgoing transitions
             * together with the full condition of each.
             *
             * @param[in] base - The base in which state and output values are printed, either 2 or 10. Defaults to 10.
             * @returns OK() and the state transition graph as text on success, an error otherwise.
             */
            Result<std::string> to_string(const u32 base = 10) const;

            /**
             * @brief Write the state transition graph to a text file, without truncating anything.
             *
             * @param[in] file_path - The file path at which to store the text representation.
             * @param[in] base - The base in which state and output values are printed, either 2 or 10. Defaults to 10.
             * @returns OK() on success, an error otherwise.
             */
            Result<std::monostate> write_txt(const std::filesystem::path& file_path, const u32 base = 10) const;
        };
    }    // namespace solve_fsm
}    // namespace hal
