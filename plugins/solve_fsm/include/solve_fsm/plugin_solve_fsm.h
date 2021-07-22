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

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "utils/fsm_transition.h"
#include "z3++.h"

#include <map>
#include <set>

namespace hal
{
    /* forward declaration */
    class Gate;
    class Netlist;
    class Net;

    class PLUGIN_API SolveFsmPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;
        void initialize() override;

        /**
         * Generates the state graph of a finite state machine and returns a mapping from each state to a vector of all its possible transitions.
         * A transition consits of a successor state and a vector of input mappings that lead to such a transition.
         *
         * @param[in] nl - Pointer to the netlist.
         * @param[in] state_reg - A vector containing all the gates of the fsm representing the state register.
         * @param[in] transition_logic - A vector containing all the gates of the fsm representing the transition_logic.
         * @param[in] initial_state - A mapping from the state registers to their initial value. If omitted the intial state will be set to 0.
         * @param[in] graph_path - Path where the transition state graph in dot format is saved.
         * @param[in] timeout - Timeout value for the sat solvers. Defaults to 600000 ms.
         * @returns A mapping from each state to all its possible transitions.
         */
        std::map<u64, std::map<u64, std::vector<std::map<u32, u8>>>> solve_fsm(Netlist* nl,
                                                  const std::vector<Gate*> state_reg,
                                                  const std::vector<Gate*> transition_logic,
                                                  const std::map<Gate*, bool> initial_state = {},
                                                  const std::string graph_path              = "",
                                                  const u32 timeout                         = 600000);

        /**
         * Generates the state graph of a finite state machine and returns a mapping from each state to a vector of all its possible successor states using a simple brute force approach.
         *
         * @param[in] nl - Pointer to the netlist.
         * @param[in] state_reg - A vector containing all the gates of the fsm representing the state register.
         * @param[in] transition_logic - A vector containing all the gates of the fsm representing the transition_logic.
         * @param[in] graph_path - Path where the transition state graph in dot format is saved.
         * @returns A mapping from each state to all its successors states.
         */
        std::map<u64, std::vector<u64>> solve_fsm_brute_force(Netlist* nl, const std::vector<Gate*> state_reg, const std::vector<Gate*> transition_logic, const std::string graph_path = "");

    private:
        std::map<Net*, Net*> find_output_net_to_input_net(const std::set<Gate*> state_reg);

        std::vector<FsmTransition>
            get_state_successors(const z3::expr& prev_state_vec, const z3::expr& next_state_vec, const z3::expr& start_state, const std::map<u32, z3::expr>& external_ids_to_expr);
        std::vector<u32> get_relevant_external_inputs(const z3::expr& state, const std::map<u32, z3::expr>& external_ids_to_expr);
        FsmTransition generate_transition_with_inputs(const z3::expr& start_state, const z3::expr& state, const std::vector<u32>& inputs, const u64 input_values);
        std::vector<FsmTransition> merge_transitions(const std::vector<FsmTransition>& transitions);

        std::string generate_dot_graph(const Netlist* nl, const std::vector<FsmTransition>& transitions, const std::vector<Gate*>& state_reg);
        std::string generate_state_transition_table(const Netlist* nl, const std::vector<FsmTransition>& transitions, const std::map<u32, z3::expr>& external_ids_to_expr);
    };
}    // namespace hal
