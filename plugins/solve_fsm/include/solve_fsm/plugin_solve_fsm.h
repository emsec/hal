#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

#include "utils/fsm_transition.h"

#include <map>
#include <set>

#include "z3++.h"

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
         * Generates the state graph of a finite state machine in dot format of a given fsm using the z3 as a sat solver.
         *
         * @param[in] nl - Pointer to the netlist.
         * @param[in] state_reg - A vector containing all the gates of the fsm representing the state register.
         * @param[in] transition_logic - A vector containing all the gates of the fsm representing the transition_logic.
         * @param[in] initial_state - A mapping from the state registers to their initial value. If omitted the intial state will be set to 0.
         * @param[in] timeout - Timeout value for the sat solvers. Defaults to 600 (unit unkown).
         * @returns The transition state graph in dot format.
         */
        std::string solve_fsm(Netlist* nl, const std::vector<Gate*> state_reg, const std::vector<Gate*> transition_logic, const std::map<Gate*, bool> initial_state = {}, const u32 timeout = 600);

    private:
        std::map<Net*, Net*> find_output_net_to_input_net(const std::set<Gate*> state_reg);
        
        std::vector<FsmTransition> get_state_successors(const z3::expr& prev_state_vec, const z3::expr& next_state_vec, const z3::expr& start_state, const std::map<u32, z3::expr>& external_ids_to_expr);
        std::vector<u32> get_relevant_external_inputs(const z3::expr& state, const std::map<u32, z3::expr>& external_ids_to_expr);
        FsmTransition generate_transition_with_inputs(const z3::expr& start_state, const z3::expr& state, const std::vector<u32>& inputs, const u64 input_values);
        std::vector<FsmTransition> merge_transitions(const std::vector<FsmTransition>& transitions);

        std::string generate_dot_graph(const Netlist* nl, const std::vector<FsmTransition>& transitions);
        std::string generate_state_transition_table(const Netlist* nl, const std::vector<FsmTransition>& transitions, const std::map<u32, z3::expr>& external_ids_to_expr);
    };
}    // namespace hal
