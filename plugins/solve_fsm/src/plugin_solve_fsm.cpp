#include "solve_fsm/plugin_solve_fsm.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <vector>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<SolveFsmPlugin>();
    }

    std::string SolveFsmPlugin::get_name() const
    {
        return std::string("solve_fsm");
    }

    std::string SolveFsmPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void SolveFsmPlugin::initialize()
    {
    }

    namespace
    {
        // generates a list of state flip flop output nets and the corresponding boolean function at their data input
        Result<std::vector<std::pair<Net*, BooleanFunction>>> generate_state_bfs(Netlist* nl, const std::vector<Gate*> state_reg, const std::vector<Gate*> transition_logic, const bool consider_control_inputs)
        {
            std::map<Net*, Net*> output_net_to_input_net;

            for (const auto& ff : state_reg)
            {
                const std::vector<GatePin*> d_pins = ff->get_type()->get_pins([](const GatePin* pin) { return pin->get_type() == PinType::data; });
                if (d_pins.size() != 1)
                {
                    return ERR("failed to create input - output mapping: currently not supporting flip-flops with multiple or no data inputs, but found " + std::to_string(d_pins.size())
                               + " for gate type " + ff->get_type()->get_name() + ".");
                }

                hal::Net* input_net;
                if (auto res = ff->get_fan_in_net(d_pins.front()); res == nullptr)
                {
                    return ERR("failed to create input - output mapping: could not get fan-in net at pin " + d_pins.front()->get_name() + " of gate " + std::to_string(ff->get_id()) + ".");
                }
                else
                {
                    input_net = res;
                }

                for (const auto& out_net : ff->get_fan_out_nets())
                {
                    output_net_to_input_net.insert({out_net, input_net});
                }
            }

            std::vector<std::pair<Net*, BooleanFunction>> state_bfs;

            const std::vector<const Gate*> subgraph_gates = {transition_logic.begin(), transition_logic.end()};
            const auto nl_dec                             = SubgraphNetlistDecorator(*nl);

            for (const auto& ff : state_reg)
            {
                const std::vector<GatePin*> d_pins = ff->get_type()->get_pins([](const GatePin* pin) { return pin->get_type() == PinType::data; });
                const GatePin* d_pin = d_pins.front();

                const std::vector<GatePin*> state_pins = ff->get_type()->get_pins([](const GatePin* pin) { return pin->get_type() == PinType::state; });
                const std::vector<GatePin*> neg_state_pins = ff->get_type()->get_pins([](const GatePin* pin) { return pin->get_type() == PinType::neg_state; });

                Net* data_net = ff->get_fan_in_net(d_pin);

                BooleanFunction bf;
                if (consider_control_inputs)
                {
                    BooleanFunction complete_bf;
                    std::string internal_state_identifier;
                    std::string internal_negated_state_identifier;

                    if (ff->get_type()->has_property(GateTypeProperty::ff))
                    {
                        const FFComponent* ff_component = ff->get_type()->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });
                        const StateComponent* state_componenet = ff->get_type()->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
                    
                        complete_bf = ff_component->get_next_state_function();
                        internal_state_identifier = state_componenet->get_state_identifier();
                        internal_negated_state_identifier = state_componenet->get_neg_state_identifier();
                    }
                    else 
                    {
                        return ERR("failed to generate boolean functions of state: gate " + ff->get_name() + " with ID " + std::to_string(ff->get_id()) + " of state register has an unhandeled type " + ff->get_type()->get_name());
                    }

                    std::cout << complete_bf << std::endl;

                    for (const auto& pin_var : complete_bf.get_variable_names())
                    {
                        // The complete Boolean function of a flip flop will contain the internal state and negated internal state.
                        // We substitute them with the outgoing state / negated state nets.
                        if (pin_var == internal_state_identifier)
                        {
                            if (state_pins.size() != 1)
                            {
                                return ERR("failed to generate boolean functions of state: found " + std::to_string(state_pins.size()) + " state pins at gate " + ff->get_name() + " with ID " + std::to_string(ff->get_id()) + ", but we expect exactly 1.");
                            }

                            complete_bf = complete_bf.substitute(internal_state_identifier, BooleanFunctionNetDecorator(*(ff->get_fan_out_net(state_pins.front()))).get_boolean_variable_name());

                            continue;
                        }

                        if (pin_var == internal_negated_state_identifier)
                        {
                            if (neg_state_pins.size() != 1)
                            {
                                return ERR("failed to generate boolean functions of state: found " + std::to_string(neg_state_pins.size()) + " neg state pins at gate " + ff->get_name() + " with ID " + std::to_string(ff->get_id()) + ", but we expect exactly 1.");
                            }

                            complete_bf = complete_bf.substitute(internal_state_identifier, BooleanFunctionNetDecorator(*(ff->get_fan_out_net(neg_state_pins.front()))).get_boolean_variable_name());

                            continue;
                        }

                        const auto pin_net = ff->get_fan_in_net(pin_var);
                        BooleanFunction pin_bf;
                        if (auto res = nl_dec.get_subgraph_function(subgraph_gates, pin_net); res.is_error())
                        {
                            return ERR_APPEND(res.get_error(),
                                            "failed to generate boolean functions of state: could not generate subgraph function for state net " + std::to_string(pin_net->get_id()) + ".");
                        }
                        else
                        {
                            pin_bf = res.get();
                        }

                        if (auto res = BooleanFunctionDecorator(pin_bf).substitute_power_ground_nets(nl); res.is_error())
                        {
                            return ERR_APPEND(res.get_error(),
                                            "failed to generate boolean functions of state: could not substitute power and ground nets in boolean funtion of net " + std::to_string(pin_net->get_id()));
                        }
                        else
                        {
                            pin_bf = res.get();
                        }

                        if (auto res = complete_bf.substitute(pin_var, pin_bf); res.is_error())
                        {
                            return ERR_APPEND(res.get_error(),
                                            "failed to generate boolean functions of state: could not substitute variable " + pin_var + " in boolean funtion of net " + std::to_string(pin_net->get_id()));
                        }
                        else
                        {
                            complete_bf = res.get();
                        }
                    }

                    std::cout << complete_bf << std::endl;

                    bf = complete_bf;
                }
                else
                {
                    if (auto res = nl_dec.get_subgraph_function(subgraph_gates, data_net); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(),
                                        "failed to generate boolean functions of state: could not generate subgraph function for state net " + std::to_string(data_net->get_id()) + ".");
                    }
                    else
                    {
                        bf = res.get();
                    }

                    if (auto res = BooleanFunctionDecorator(bf).substitute_power_ground_nets(nl); res.is_error())
                    {
                        return ERR_APPEND(res.get_error(),
                                        "failed to generate boolean functions of state: could not substitute power and ground nets in boolean funtion of net " + std::to_string(data_net->get_id()));
                    }
                    else
                    {
                        bf = res.get();
                    }
                }

                bf.simplify();

                const auto var_names = bf.get_variable_names();

                // in the transition logic expressions of the next state bits we substitue the output nets of the state flip-flops with their (negated) input net.
                for (const auto& [out, in] : output_net_to_input_net)
                {
                    // check whether output net is part of the expression
                    if (var_names.find(BooleanFunctionNetDecorator(*out).get_boolean_variable_name()) == var_names.end())
                    {
                        continue;
                    }

                    auto in_bf = BooleanFunctionNetDecorator(*in).get_boolean_variable();

                    // check for multidriven nets
                    if (out->get_sources().size() != 1)
                    {
                        return ERR("failed to generate boolean functions of state: found multi driven net " + std::to_string(out->get_id()) + ".");
                    }

                    // negate if the output stems from the negated state output
                    const GatePin* src_pin = out->get_sources().front()->get_pin();
                    if (src_pin->get_type() == PinType::neg_state)
                    {
                        in_bf = ~in_bf;
                    }

                    auto res = bf.substitute(BooleanFunctionNetDecorator(*out).get_boolean_variable_name(), in_bf);

                    if (res.is_error())
                    {
                        return ERR("failed to generate boolean functions of state: unable to replace out net " + std::to_string(out->get_id()) + " with in net " + std::to_string(in->get_id()) + ".");
                    }

                    bf = res.get();
                }

                state_bfs.push_back({data_net, bf});
            }

            return OK(state_bfs);
        }

        // takes a map of unconditional transitions and reconstructs the conditions under which each condition is taken
        Result<std::map<u64, std::map<u64, BooleanFunction>>> generate_conditional_transitions(const std::vector<std::pair<Net*, BooleanFunction>>& state_bfs,
                                                                                               const std::map<u64, std::set<u64>>& transitions)
        {
            // generate all transitions that are reachable from the inital state.
            std::map<u64, std::map<u64, BooleanFunction>> conditional_transitions;

            // for all possible and previously found successor states we build the condition to reach them
            for (const auto& [prev, successors] : transitions)
            {
                // this builds a mapping for all the output net variables of the state vector to the current starting state
                std::map<std::string, BooleanFunction> prev_mapping;
                for (u32 i = 0; i < state_bfs.size(); i++)
                {
                    prev_mapping.insert(
                        {BooleanFunctionNetDecorator(*(state_bfs.at(i).first)).get_boolean_variable_name(), ((prev >> i) & 1) ? BooleanFunction::Const(1, 1) : BooleanFunction::Const(0, 1)});
                }

                for (const auto& suc : successors)
                {
                    // this all the boolean functions of incoming data nets to the state vector either vanilla incase the corresponding state bit is 1 in the successor state or negated incase the state bit is 0 in the successor state.
                    BooleanFunction condition;

                    for (u32 i = 0; i < state_bfs.size(); i++)
                    {
                        auto next_state_bit_bf = ((suc >> i) & 1) ? state_bfs.at(i).second : BooleanFunction::Not(state_bfs.at(i).second.clone(), 1).get();

                        if (condition.is_empty())
                        {
                            condition = next_state_bit_bf;
                        }
                        else
                        {
                            condition = BooleanFunction::And(std::move(condition), std::move(next_state_bit_bf), 1).get();
                        }
                    }

                    // replace all the variables of the previous state with their real values for our current state n and simplify.
                    condition = condition.substitute(prev_mapping).get();

                    // we are left with a condition that only includes the inputs to the fsm that needs to be fullfilled to reach the successor state from state n.
                    condition = condition.simplify();

                    conditional_transitions[prev].insert({suc, condition});
                }
            }

            return OK(conditional_transitions);
        }

    }    // namespace

    Result<std::map<u64, std::map<u64, BooleanFunction>>>
        SolveFsmPlugin::solve_fsm_brute_force(Netlist* nl, const std::vector<Gate*> state_reg, const std::vector<Gate*> transition_logic, const std::string graph_path)
    {
        const u32 state_size = state_reg.size();
        if (state_size > 64)
        {
            return ERR("failed to solve fsm: Currently only supports fsm with up to 64 state flip-flops but got " + std::to_string(state_size) + ".");
        }

        // extract Boolean functions for each state flip-flop
        const auto state_bfs_res = generate_state_bfs(nl, state_reg, transition_logic, true);
        if (state_bfs_res.is_error())
        {
            return ERR_APPEND(state_bfs_res.get_error(), "failed to solve fsm: unable to generate Boolean functions for state.");
        }
        const std::vector<std::pair<Net*, BooleanFunction>> state_bfs = state_bfs_res.get();

        // bitvector including all the functions to calculate the next state
        BooleanFunction next_state_vec = state_bfs.front().second;
        for (u32 i = 1; i < state_reg.size(); i++)
        {
            next_state_vec = BooleanFunction::Concat(state_bfs.at(i).second.clone(), std::move(next_state_vec), next_state_vec.size() + 1).get();
        }

        std::map<u64, std::set<u64>> all_transitions;

        for (u64 state = 0; state < (u64(1) << state_size); state++)
        {
            // generate state map
            std::map<std::string, BooleanFunction> var_to_val;
            for (u32 state_index = 0; state_index < state_size; state_index++)
            {
                std::string var     = BooleanFunctionNetDecorator(*(state_bfs.at(state_index).first)).get_boolean_variable_name();
                BooleanFunction val = ((state >> state_index) & 0x1) ? BooleanFunction::Const(1, 1) : BooleanFunction::Const(0, 1);
                var_to_val.insert({var, val});
            }

            const auto sub_res = next_state_vec.substitute(var_to_val);
            if (sub_res.is_error())
            {
                return ERR_APPEND(sub_res.get_error(), "failed to solve fsm: unable to substitute variables in next state vec.");
            }

            const auto state_bf = sub_res.get().simplify();
            const auto inputs   = utils::to_vector(state_bf.get_variable_names());

            // brute force over all external inputs
            for (u64 input_val = 0; input_val < (u64(1) << inputs.size()); input_val++)
            {
                // generate input map
                std::unordered_map<std::string, std::vector<BooleanFunction::Value>> input_mapping;
                for (u32 input_index = 0; input_index < inputs.size(); input_index++)
                {
                    std::string input_var      = inputs.at(input_index);
                    BooleanFunction::Value val = ((input_val >> input_index) & 0x1) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO;
                    input_mapping.insert({input_var, {val}});
                }

                const auto& eval_res = state_bf.evaluate(input_mapping);
                if (sub_res.is_error())
                {
                    return ERR_APPEND(sub_res.get_error(), "failed to solve fsm: unable to evaluate next state function.");
                }

                const auto eval = eval_res.get();

                if (eval.front() == BooleanFunction::Value::X)
                {
                    return ERR("failed to solve fsm: evaluating state function resulted in X state.");
                }

                const u64 suc_state = BooleanFunction::to_u64(eval).get();
                all_transitions[state].insert(suc_state);
            }
        }

        const auto conditional_transitions = generate_conditional_transitions(state_bfs, all_transitions).get();

        /* DEBUG PRINTING */
        for (const auto& [org, successors] : conditional_transitions)
        {
            std::cout << org << ": " << std::endl;
            for (const auto& [suc, condition] : successors)
            {
                std::cout << "\t" << suc << ": " << condition.to_string() << std::endl;
            }
        }
        /* END DEBUG PRINTING */

        if (auto graph_res = generate_dot_graph(state_reg, conditional_transitions, graph_path); graph_res.is_error())
        {
            return ERR_APPEND(graph_res.get_error(), "failed to solve fsm: unable to generate dot graph.");
        }

        return OK(conditional_transitions);
    }

    Result<std::map<u64, std::map<u64, BooleanFunction>>> SolveFsmPlugin::solve_fsm(Netlist* nl,
                                                                                    const std::vector<Gate*> state_reg,
                                                                                    const std::vector<Gate*> transition_logic,
                                                                                    const std::map<Gate*, bool> initial_state,
                                                                                    const std::string graph_path,
                                                                                    const u32 timeout)
    {
        const u32 state_size = state_reg.size();
        if (state_size > 64)
        {
            return ERR("failed to solve fsm: Currently only supports fsm with up to 64 state flip-flops but got " + std::to_string(state_size) + ".");
        }

        // extract Boolean functions for each state flip-flop
        const auto state_bfs_res = generate_state_bfs(nl, state_reg, transition_logic, true);
        if (state_bfs_res.is_error())
        {
            return ERR_APPEND(state_bfs_res.get_error(), "failed to solve fsm: unable to generate Boolean functions for state.");
        }
        const std::vector<std::pair<Net*, BooleanFunction>> state_bfs = state_bfs_res.get();

        BooleanFunction prev_state_vec = BooleanFunctionNetDecorator(*(state_bfs.front().first)).get_boolean_variable();
        BooleanFunction next_state_vec = state_bfs.front().second;
        for (u32 i = 1; i < state_reg.size(); i++)
        {
            // bitvector representing the previous state
            prev_state_vec = BooleanFunction::Concat(BooleanFunctionNetDecorator(*(state_bfs.at(i).first)).get_boolean_variable(), std::move(prev_state_vec), i + 1).get();

            // bitvector including all the functions to calculate the next state
            next_state_vec = BooleanFunction::Concat(state_bfs.at(i).second.clone(), std::move(next_state_vec), i + 1).get();
        }

        // generate initial state
        u64 initial_state_num = 0;
        if (!initial_state.empty())
        {
            for (const auto& gate : state_reg)
            {
                if (initial_state.find(gate) == initial_state.end())
                {
                    return ERR("failed to solve fsm: Unable to find intial value for gate " + std::to_string(gate->get_id()) + " in the provided initial state map.");
                }

                initial_state_num = initial_state_num << 1;
                initial_state_num += initial_state.at(gate);
            }
        }

        // generate all transitions that are reachable from the inital state.
        std::map<u64, std::set<u64>> all_transitions;

        std::deque<u64> q;
        std::unordered_set<u64> visited;

        q.push_back(initial_state_num);

        while (!q.empty())
        {
            std::vector<u64> successor_states;

            u64 n = q.front();
            q.pop_front();

            if (visited.find(n) != visited.end())
            {
                continue;
            }
            visited.insert(n);

            // generate new transitions and add them to the queue
            SMT::Solver s;

            // set prev_state_vec to starting state
            s = s.with_constraint(SMT::Constraint{prev_state_vec.clone(), BooleanFunction::Const(n, state_size)});

            while (true)
            {
                if (auto res = s.query(SMT::QueryConfig().with_model_generation().with_timeout(timeout)); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "failed to solve fsm: failed to querry SMT solver for state " + std::to_string(n) + ".");
                }
                else
                {
                    auto s_res = res.get();

                    if (s_res.is_unsat())
                    {
                        break;
                    }

                    if (s_res.is_unknown())
                    {
                        return ERR("failed to solve fsm: received an unknown solver result for state " + std::to_string(n) + ".");
                    }

                    auto m       = s_res.model.value();
                    auto suc     = m.evaluate(next_state_vec).get();
                    auto suc_num = 0;

                    // a constant (numeral) successor state
                    if (suc.is_constant())
                    {
                        suc_num = suc.get_constant_value_u64().get();
                    }
                    // a successor state that includes boolean functions (for example in form of input variables)
                    else
                    {
                        // to resolve such a successor state, we simpply set all variables left in the state to zero (which is one possible solution) and continue to search for more valid solutions
                        std::unordered_map<std::string, std::vector<BooleanFunction::Value>> zero_mapping;
                        for (const auto& var : suc.get_variable_names())
                        {
                            zero_mapping.insert({var, {BooleanFunction::Value::ZERO}});
                        }

                        if (auto eval_res = suc.evaluate(zero_mapping); eval_res.is_error())
                        {
                            return ERR_APPEND(eval_res.get_error(), "failed to solve fsm: could not evaluate successor state to constant.");
                        }
                        else
                        {
                            suc_num = BooleanFunction::to_u64(eval_res.get()).get();
                        }
                    }

                    q.push_back(suc_num);
                    all_transitions[n].insert(suc_num);
                    s = s.with_constraint(SMT::Constraint(BooleanFunction::Not(BooleanFunction::Eq(next_state_vec.clone(), BooleanFunction::Const(suc_num, suc.size()), 1).get(), 1).get()));
                }
            }
        }

        const auto conditional_transitions = generate_conditional_transitions(state_bfs, all_transitions).get();

        /* DEBUG PRINTING */
        for (const auto& [org, successors] : conditional_transitions)
        {
            std::cout << org << ": " << std::endl;
            for (const auto& [suc, condition] : successors)
            {
                std::cout << "\t" << suc << ": " << condition.to_string() << std::endl;
            }
        }
        /* END DEBUG PRINTING */

        if (auto graph_res = generate_dot_graph(state_reg, conditional_transitions, graph_path); graph_res.is_error())
        {
            return ERR_APPEND(graph_res.get_error(), "failed to solve fsm: unable to generate dot graph.");
        }

        return OK(conditional_transitions);
    }

    Result<std::string> SolveFsmPlugin::generate_dot_graph(const std::vector<Gate*>& state_reg,
                                                           const std::map<u64, std::map<u64, BooleanFunction>>& transitions,
                                                           const std::string& graph_path,
                                                           const u32 max_condition_length,
                                                           const u32 base)
    {
        // auto generate_legend = [](const std::vector<Gate*>& reg) -> std::string {
        //     std::string name_str = "";
        //     for (const auto& ff : reg)
        //     {
        //         name_str = name_str + ff->get_name();
        //         name_str += ", ";
        //     }

        //     name_str = name_str.substr(0, name_str.size() - 2);

        //     return "legend[label=\"" + name_str + +"\", shape=box]" + "\n";
        // };

        std::string graph_str = "digraph {\n";

        // TODO save this information somewhere else maybe in a results.txt similar to the dataflow
        // graph_str += generate_legend(state_reg);

        for (const auto& [org, successors] : transitions)
        {
            for (const auto& [suc, cond] : successors)
            {
                std::string start_name;
                std::string end_name;

                switch (base)
                {
                    case 2:
                        start_name = std::bitset<64>(org).to_string().substr(64 - state_reg.size(), 64);
                        end_name   = std::bitset<64>(suc).to_string().substr(64 - state_reg.size(), 64);
                        break;
                    case 10:
                        start_name = std::to_string(org);
                        end_name   = std::to_string(suc);
                        break;
                    default:
                        return ERR("failed to generate dot graph: base " + std::to_string(base) + "not implemented.");
                }

                graph_str +=
                    start_name + " -> " + end_name + "[label=\"" + cond.to_string().substr(0, max_condition_length) + "\", weight=\"" + cond.to_string().substr(0, max_condition_length) + "\"];\n";
                ;
            }
        }

        graph_str += "}";

        // write to file
        if (!graph_path.empty())
        {
            std::ofstream ofs(graph_path);
            if (!ofs.is_open())
            {
                return ERR("failed to generate dot graph: could not open file " + graph_path + " for writing.");
            }
            ofs << graph_str;
            ofs.close();
        }

        return OK(graph_str);
    }

    /*
    Result<std::string> SolveFsmPlugin::generate_dot_graph(const std::vector<Gate*>& state_reg,
                                                           const std::map<u64, std::set<u64>>& transitions,
                                                           const std::string& graph_path,
                                                           const u32 max_condition_length,
                                                           const u32 base)
    {
        std::map<u64, std::map<u64, BooleanFunction>> dummy_conditional_transitions;

        for (const auto& [org, successors] : transitions)
        {
            for (const auto& suc : successors)
            {
                dummy_conditional_transitions[org].insert({suc, BooleanFunction()});
            }
        }

        return generate_dot_graph(state_reg, dummy_conditional_transitions, graph_path, max_condition_length, base);
    }
    */

}    // namespace hal