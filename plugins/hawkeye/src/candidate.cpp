#include "hawkeye/candidate.h"

#include "hal_core/netlist/gate.h"

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

    }    // namespace hawkeye
}    // namespace hal