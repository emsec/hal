#include "hawkeye/round_candidate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"

namespace hal
{
    namespace hawkeye
    {
        namespace
        {
            void copy_in_eps_of_gate(Netlist* new_nl, const Gate* old_g, Gate* new_g, std::set<Net*>* add_to_set = nullptr)
            {
                for (const auto* in_ep : old_g->get_fan_in_endpoints())
                {
                    Net* old_n = in_ep->get_net();
                    Net* new_n;
                    if (new_n = new_nl->get_net_by_id(old_n->get_id()); new_n == nullptr)
                    {
                        new_n = new_nl->create_net(old_n->get_id(), old_n->get_name());
                    }
                    new_n->add_destination(new_g, in_ep->get_pin());

                    if (add_to_set)
                    {
                        add_to_set->insert(new_n);
                    }
                }
            }

            void copy_out_eps_of_gate(Netlist* new_nl, const Gate* old_g, Gate* new_g, std::set<Net*>* add_to_set = nullptr)
            {
                for (const auto* out_ep : old_g->get_fan_out_endpoints())
                {
                    Net* old_n = out_ep->get_net();
                    Net* new_n;
                    if (new_n = new_nl->get_net_by_id(old_n->get_id()); new_n == nullptr)
                    {
                        new_n = new_nl->create_net(old_n->get_id(), old_n->get_name());
                    }
                    new_n->add_source(new_g, out_ep->get_pin());

                    if (add_to_set)
                    {
                        add_to_set->insert(new_n);
                    }
                }
            }
        }    // namespace

        Result<std::unique_ptr<RoundCandidate>> RoundCandidate::from_register_candidate(RegisterCandidate* candidate)
        {
            std::set<Gate*> state_logic;
            std::set<Net*> state_inputs, state_outputs, control_inputs, other_inputs;

            log_info("hawkeye", "start isolating state logic...");
            auto start = std::chrono::system_clock::now();

            const auto& state_input_reg  = candidate->get_input_reg();
            const auto& state_output_reg = candidate->get_output_reg();

            // DFS from output reg backwards
            for (const auto* out_ff : state_output_reg)
            {
                auto ff_data_predecessors = out_ff->get_predecessors([](const GatePin* p, const Endpoint* _) { return p->get_type() == PinType::data; });

                if (ff_data_predecessors.size() != 1)
                {
                    // FF can only have one predecessor for data input
                    continue;
                }
                const auto* pred_ep   = ff_data_predecessors.at(0);
                auto* first_comb_gate = pred_ep->get_gate();
                if (!first_comb_gate->get_type()->has_property(GateTypeProperty::combinational))
                {
                    continue;
                }
                state_outputs.insert(pred_ep->get_net());

                std::unordered_set<Gate*> visited;
                std::vector<Gate*> stack = {first_comb_gate};
                std::vector<Gate*> previous;
                while (!stack.empty())
                {
                    auto* current_gate = stack.back();

                    // pop stack if last gate on stack has been dealt with completely
                    if (!previous.empty() && previous.back() == current_gate)
                    {
                        stack.pop_back();
                        previous.pop_back();
                        continue;
                    }

                    visited.insert(current_gate);

                    // expand towards predecessors
                    bool added = false;
                    for (auto* next_predecessor : current_gate->get_predecessors())
                    {
                        auto* predecessor_gate = next_predecessor->get_gate();
                        if (predecessor_gate->get_type()->has_property(GateTypeProperty::ff))
                        {
                            // if predecessor is part of input state reg, fill set of next state logic
                            if (state_input_reg.find(predecessor_gate) != state_input_reg.end())
                            {
                                state_inputs.insert(next_predecessor->get_net());
                                state_logic.insert(current_gate);
                                state_logic.insert(previous.begin(), previous.end());
                            }
                        }
                        else if (predecessor_gate->get_type()->has_property(GateTypeProperty::combinational))
                        {
                            if (visited.find(predecessor_gate) == visited.end())
                            {
                                // add only combinational predecessors to stack
                                stack.push_back(predecessor_gate);
                                added = true;
                            }
                        }
                    }

                    if (added)
                    {
                        // push current gate to previous if progress was made
                        previous.push_back(current_gate);
                    }
                    else
                    {
                        // otherwise pop last element from stack as it has been dealt with already
                        stack.pop_back();
                    }
                }
            }

            auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully isolated state logic in {} seconds", duration_in_seconds);

            log_info("hawkeye", "start identifying control inputs...");
            start = std::chrono::system_clock::now();

            std::set<Net*> visited;
            for (auto* gate : state_logic)
            {
                // determine control inputs and other inputs to the candidate
                for (auto* in_net : gate->get_fan_in_nets())
                {
                    if (visited.find(in_net) != visited.end())
                    {
                        continue;
                    }

                    visited.insert(in_net);

                    if (in_net->get_num_of_sources() != 1)
                    {
                        continue;
                    }

                    if (state_inputs.find(in_net) != state_inputs.end())
                    {
                        continue;
                    }

                    auto* src_gate = in_net->get_sources().at(0)->get_gate();
                    if (state_logic.find(src_gate) != state_logic.end())
                    {
                        continue;
                    }

                    u32 num_state_destinations = in_net->get_num_of_destinations([&state_logic](const Endpoint* ep) { return state_logic.find(ep->get_gate()) != state_logic.end(); });
                    if (num_state_destinations > candidate->get_size() / 2)
                    {
                        control_inputs.insert(in_net);
                    }
                    else
                    {
                        other_inputs.insert(in_net);
                    }
                }
            }

            duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully identified control inputs in {} seconds", duration_in_seconds);

            // copy partial netlist
            auto round_cand       = std::make_unique<RoundCandidate>();
            round_cand->m_netlist = std::move(netlist_factory::create_netlist(candidate->get_netlist()->get_gate_library()));
            auto* copied_nl       = round_cand->m_netlist.get();

            round_cand->m_size = candidate->get_size();

            for (const auto* g : state_input_reg)
            {
                auto* new_g = copied_nl->create_gate(g->get_id(), g->get_type(), g->get_name());
                round_cand->m_in_reg.insert(new_g);

                copy_out_eps_of_gate(copied_nl, g, new_g);    // only fan-out EPs for state input register
            }

            for (const auto* g : state_logic)
            {
                auto* new_g = copied_nl->create_gate(g->get_id(), g->get_type(), g->get_name());
                new_g->set_data_map(g->get_data_map());    // take care of LUT INIT strings
                round_cand->m_state_logic.insert(new_g);

                copy_in_eps_of_gate(copied_nl, g, new_g);
                copy_out_eps_of_gate(copied_nl, g, new_g);
            }

            if (state_input_reg != state_output_reg)
            {
                for (const auto* g : state_output_reg)
                {
                    auto* new_g = copied_nl->create_gate(g->get_id(), g->get_type(), g->get_name());
                    round_cand->m_out_reg.insert(new_g);

                    copy_in_eps_of_gate(copied_nl, g, new_g);    // only fan-in EPs for state output register
                }
            }
            else
            {
                // create separate FF instances for state output register so that input and output register are distinct
                for (const auto* g : state_output_reg)
                {
                    // only differences: do not enforce ID (already taken by in_reg FF) and append suffix to name
                    auto* new_g = copied_nl->create_gate(g->get_type(), g->get_name() + "_OUT");
                    round_cand->m_out_reg.insert(new_g);

                    copy_in_eps_of_gate(copied_nl, g, new_g);    // only fan-in EPs for state output register
                }
            }

            for (const auto* state_in_net : state_inputs)
            {
                round_cand->m_state_inputs.insert(copied_nl->get_net_by_id(state_in_net->get_id()));
            }

            for (const auto* state_out_net : state_outputs)
            {
                round_cand->m_state_outputs.insert(copied_nl->get_net_by_id(state_out_net->get_id()));
            }

            for (const auto* control_net : control_inputs)
            {
                round_cand->m_control_inputs.insert(copied_nl->get_net_by_id(control_net->get_id()));
            }

            for (const auto* other_net : other_inputs)
            {
                round_cand->m_other_inputs.insert(copied_nl->get_net_by_id(other_net->get_id()));
            }

            auto nl_graph_res = graph_algorithm::NetlistGraph::from_netlist(copied_nl);
            if (nl_graph_res.is_error())
            {
                return ERR(nl_graph_res.get_error());
            }
            round_cand->m_graph = std::move(nl_graph_res.get());

            // DFS from input reg forwards
            std::map<Gate*, u32> gate_to_longest_distance;
            for (auto* in_ff : round_cand->m_in_reg)
            {
                std::vector<Gate*> stack = {in_ff};
                std::vector<Gate*> previous;
                while (!stack.empty())
                {
                    auto* current_gate = stack.back();

                    // pop stack if last gate on stack has been dealt with completely
                    if (!previous.empty() && previous.back() == current_gate)
                    {
                        stack.pop_back();
                        previous.pop_back();
                        continue;
                    }

                    round_cand->m_input_ffs_of_gate[current_gate].insert(in_ff);
                    round_cand->m_gates_reached_by_input_ff[in_ff].insert(current_gate);

                    // expand towards successors
                    bool added = false;
                    for (auto* next_successor : current_gate->get_successors())
                    {
                        auto* successor_gate = next_successor->get_gate();
                        if (successor_gate->get_type()->has_property(GateTypeProperty::ff))
                        {
                            // if successor is part of output state reg, fill set of gates reached by input FF
                            if (round_cand->m_out_reg.find(successor_gate) != round_cand->m_out_reg.end())
                            {
                                round_cand->m_input_ffs_of_gate[successor_gate].insert(in_ff);
                                round_cand->m_gates_reached_by_input_ff[in_ff].insert(successor_gate);
                            }
                        }
                        else if (successor_gate->get_type()->has_property(GateTypeProperty::combinational))
                        {
                            // if successor is part of next state logic, add gate to stack
                            if (round_cand->m_state_logic.find(successor_gate) != round_cand->m_state_logic.end())
                            {
                                stack.push_back(successor_gate);
                                added = true;

                                u32 current_distance = previous.size() + 1;
                                if (const auto dist_it = gate_to_longest_distance.find(successor_gate); dist_it != gate_to_longest_distance.end())
                                {
                                    u32 stored_distance = dist_it->second;
                                    if (stored_distance < current_distance)
                                    {
                                        gate_to_longest_distance[successor_gate] = current_distance;
                                    }
                                }
                                else
                                {
                                    gate_to_longest_distance[successor_gate] = current_distance;
                                }
                            }
                        }
                    }

                    if (added)
                    {
                        // push current gate to previous if progress was made
                        previous.push_back(current_gate);
                    }
                    else
                    {
                        // otherwise pop last element from stack as it has been dealt with already
                        stack.pop_back();
                    }
                }
            }

            // invert gate_to_longest_distance map to fill m_longest_distance_to_gate
            for (const auto& [gate, distance] : gate_to_longest_distance)
            {
                round_cand->m_longest_distance_to_gate[distance].insert(gate);
            }

            return OK(std::move(round_cand));
        }

        Netlist* RoundCandidate::get_netlist() const
        {
            return m_netlist.get();
        }

        graph_algorithm::NetlistGraph* RoundCandidate::get_graph() const
        {
            return m_graph.get();
        }

        u32 RoundCandidate::get_size() const
        {
            return m_size;
        }

        const std::set<Gate*>& RoundCandidate::get_input_reg() const
        {
            return m_in_reg;
        }

        const std::set<Gate*>& RoundCandidate::get_output_reg() const
        {
            return m_out_reg;
        }

        const std::set<Gate*>& RoundCandidate::get_state_logic() const
        {
            return m_state_logic;
        }

        const std::set<Net*>& RoundCandidate::get_state_inputs() const
        {
            return m_state_inputs;
        }

        const std::set<Net*>& RoundCandidate::get_control_inputs() const
        {
            return m_control_inputs;
        }

        const std::set<Net*>& RoundCandidate::get_other_inputs() const
        {
            return m_other_inputs;
        }

        const std::set<Net*>& RoundCandidate::get_state_outputs() const
        {
            return m_state_outputs;
        }

        const std::map<Gate*, std::set<Gate*>>& RoundCandidate::get_input_ffs_of_gate() const
        {
            return m_input_ffs_of_gate;
        }

        const std::map<u32, std::set<Gate*>>& RoundCandidate::get_longest_distance_to_gate() const
        {
            return m_longest_distance_to_gate;
        }
    }    // namespace hawkeye
}    // namespace hal