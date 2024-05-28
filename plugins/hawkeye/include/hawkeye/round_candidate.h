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

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/defines.h"
#include "hawkeye/register_candidate.h"

namespace hal
{
    namespace hawkeye
    {
        class RoundCandidate
        {
        public:
            RoundCandidate()  = default;
            ~RoundCandidate() = default;

            /**
             * Computes a state candidate from the previously identified register candidate.
             * The netlist of this candidate will be a partial copy of the original netlist, comprising only the gates belonging to the registers and the logic computing the next state.
             * 
             * @param[in] candidate - The register candidate.
             * @returns The state candidate on success, an error otherwise.
             */
            static Result<std::unique_ptr<RoundCandidate>> from_register_candidate(RegisterCandidate* candidate);

            /**
             * Get the netlist of the state candidate. The netlist will be a partial copy of the netlist of the register candidate.
             * 
             * @returns The netlist.
             */
            Netlist* get_netlist() const;

            /**
             * Get the netlist graph of the state candidate.
             * 
             * @returns The netlist graph.
             */
            graph_algorithm::NetlistGraph* get_graph() const;

            /**
             * Get the size of the candidate, i.e., the width of its registers.
             * 
             * @returns The size of the candidate.
             */
            u32 get_size() const;

            /**
             * Get the candidate's input register.
             * 
             * @returns The input register of the candidate.
             */
            const std::set<Gate*>& get_input_reg() const;

            /**
             * Get the candidate's output register.
             * 
             * @returns The output register of the candidate.
             */
            const std::set<Gate*>& get_output_reg() const;

            /**
             * Get the candidate's combinational logic computing the next state.
             * 
             * @returns The state logic of the candidate.
             */
            const std::set<Gate*>& get_state_logic() const;

            /**
             * Get the candidate's state inputs to the logic computing the next state.
             * 
             * @returns The state inputs of the candidate.
             */
            const std::set<Net*>& get_state_inputs() const;

            /**
             * Get the candidate's control inputs to the logic computing the next state.
             * 
             * @returns The control inputs of the candidate.
             */
            const std::set<Net*>& get_control_inputs() const;

            /**
             * Get the candidate's other inputs to the logic computing the next state.
             * 
             * @returns The other inputs of the candidate.
             */
            const std::set<Net*>& get_other_inputs() const;

            /**
             * Get the candidate's state outputs from the logic computing the next state.
             * 
             * @returns The state outputs of the candidate.
             */
            const std::set<Net*>& get_state_outputs() const;

            /**
             * Get a map from each combinational gate of the round function to all the input flip-flops it depends on.
             * 
             * @returns A map from gates to sets of input flip-flops.
             */
            const std::map<Gate*, std::set<Gate*>>& get_input_ffs_of_gate() const;

            /**
             * Get a map from an integer distance to all gates that are reachable within at most that distance when starting at any input flip-flop.
             * 
             * @returns A map from longest distance to a set of gates being reachable in at most that distance.
             */
            const std::map<u32, std::set<Gate*>>& get_longest_distance_to_gate() const;

        private:
            /**
             * The netlist to which the candidate belongs.
             */
            std::unique_ptr<Netlist> m_netlist;

            /**
             * The netlist to which the candidate belongs.
             */
            std::unique_ptr<graph_algorithm::NetlistGraph> m_graph;

            /**
             * The bit-size of the candidate.
             */
            u32 m_size;

            /**
             * The candidate input register.
             */
            std::set<Gate*> m_in_reg;

            /**
             * The candidate output register. May be equal to `m_in_reg` for round-based implementations.
             */
            std::set<Gate*> m_out_reg;

            /**
             * The combinational logic computing the next state.
             */
            std::set<Gate*> m_state_logic;

            /**
             * The state inputs to the combinational logic computing the next state.
             */
            std::set<Net*> m_state_inputs;

            /**
             * The control inputs to the combinational logic computing the next state.
             */
            std::set<Net*> m_control_inputs;

            /**
             * All other inputs to the combinational logic computing the next state.
             */
            std::set<Net*> m_other_inputs;

            /**
             * The state outputs from the combinational logic computing the next state.
             */
            std::set<Net*> m_state_outputs;

            std::map<Gate*, std::set<Gate*>> m_input_ffs_of_gate;

            std::map<u32, std::set<Gate*>> m_longest_distance_to_gate;
        };
    }    // namespace hawkeye
}    // namespace hal