#include "hawkeye/candidate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
namespace hal
{
    namespace hawkeye
    {
        Candidate::Candidate(const std::set<Gate*>& round_reg) : m_in_reg(round_reg), m_out_reg(round_reg)
        {
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = true;
        }

        Candidate::Candidate(std::set<Gate*>&& round_reg) : m_in_reg(std::move(round_reg))
        {
            m_out_reg        = m_in_reg;
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = true;
        }

        Candidate::Candidate(const std::set<Gate*>& in_reg, const std::set<Gate*>& out_reg) : m_in_reg(in_reg), m_out_reg(out_reg)
        {
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = m_in_reg == m_out_reg;
        }

        Candidate::Candidate(std::set<Gate*>&& in_reg, std::set<Gate*>&& out_reg) : m_in_reg(std::move(in_reg)), m_out_reg(std::move(out_reg))
        {
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = m_in_reg == m_out_reg;
        }

        bool Candidate::operator==(const Candidate& rhs) const
        {
            return (this->m_size == rhs.m_size) && (this->m_in_reg == rhs.m_in_reg) && (!m_is_round_based & (this->m_out_reg == rhs.m_out_reg));
        }

        bool Candidate::operator<(const Candidate& rhs) const
        {
            return (this->m_size > rhs.m_size) || (this->m_size == rhs.m_size && this->m_in_reg > rhs.m_in_reg)
                   || (!m_is_round_based & (this->m_size == rhs.m_size && this->m_in_reg == rhs.m_in_reg && this->m_out_reg > rhs.m_out_reg));
        }

        Netlist* Candidate::get_netlist() const
        {
            return m_netlist;
        }

        u32 Candidate::get_size() const
        {
            return m_size;
        }

        bool Candidate::is_round_based() const
        {
            return m_is_round_based;
        }

        const std::set<Gate*>& Candidate::get_input_reg() const
        {
            return m_in_reg;
        }

        const std::set<Gate*>& Candidate::get_output_reg() const
        {
            return m_out_reg;
        }

        const std::set<Gate*>& Candidate::get_state_logic() const
        {
            return m_state_logic;
        }

        const std::set<Net*>& Candidate::get_state_inputs() const
        {
            return m_state_inputs;
        }

        const std::set<Net*>& Candidate::get_control_inputs() const
        {
            return m_control_inputs;
        }

        const std::set<Net*>& Candidate::get_other_inputs() const
        {
            return m_other_inputs;
        }

        const std::set<Net*>& Candidate::get_state_outputs() const
        {
            return m_state_outputs;
        }

        void Candidate::isolate_round_function()
        {
            log_info("hawkeye", "start isolating state logic...");
            auto start = std::chrono::system_clock::now();

            for (const auto* out_ff : m_out_reg)
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
                m_state_outputs.insert(pred_ep->get_net());

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
                            if (m_in_reg.find(predecessor_gate) != m_in_reg.end())
                            {
                                m_state_inputs.insert(next_predecessor->get_net());
                                m_state_logic.insert(current_gate);
                                m_state_logic.insert(previous.begin(), previous.end());
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
            for (auto* gate : m_state_logic)
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

                    if (m_state_inputs.find(in_net) != m_state_inputs.end())
                    {
                        continue;
                    }

                    auto* src_gate = in_net->get_sources().at(0)->get_gate();
                    if (m_state_logic.find(src_gate) != m_state_logic.end())
                    {
                        continue;
                    }

                    u32 num_state_destinations = in_net->get_num_of_destinations([this](const Endpoint* ep) { return this->m_state_logic.find(ep->get_gate()) != m_state_logic.end(); });
                    if (num_state_destinations > m_size / 2)
                    {
                        m_control_inputs.insert(in_net);
                        continue;
                    }

                    m_other_inputs.insert(in_net);
                }
            }

            duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
            log_info("hawkeye", "successfully identified control inputs in {} seconds", duration_in_seconds);
        }

    }    // namespace hawkeye
}    // namespace hal