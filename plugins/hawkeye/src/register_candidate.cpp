#include "hawkeye/register_candidate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

#include <algorithm>

namespace hal
{
    namespace hawkeye
    {
        RegisterCandidate::RegisterCandidate(const std::set<Gate*>& round_reg) : m_in_reg(round_reg), m_out_reg(round_reg)
        {
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = true;
            m_in_reg_ids     = get_sorted_ids(m_in_reg);
            m_out_reg_ids    = m_in_reg_ids;
        }

        RegisterCandidate::RegisterCandidate(std::set<Gate*>&& round_reg) : m_in_reg(std::move(round_reg))
        {
            m_out_reg        = m_in_reg;
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = true;
            m_in_reg_ids     = get_sorted_ids(m_in_reg);
            m_out_reg_ids    = m_in_reg_ids;
        }

        RegisterCandidate::RegisterCandidate(const std::set<Gate*>& in_reg, const std::set<Gate*>& out_reg) : m_in_reg(in_reg), m_out_reg(out_reg)
        {
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = m_in_reg == m_out_reg;
            m_in_reg_ids     = get_sorted_ids(m_in_reg);
            m_out_reg_ids    = get_sorted_ids(m_out_reg);
        }

        RegisterCandidate::RegisterCandidate(std::set<Gate*>&& in_reg, std::set<Gate*>&& out_reg) : m_in_reg(std::move(in_reg)), m_out_reg(std::move(out_reg))
        {
            m_size           = m_out_reg.size();
            m_netlist        = (*m_out_reg.begin())->get_netlist();
            m_is_round_based = m_in_reg == m_out_reg;
            m_in_reg_ids     = get_sorted_ids(m_in_reg);
            m_out_reg_ids    = get_sorted_ids(m_out_reg);
        }

        std::vector<u32> RegisterCandidate::get_sorted_ids(const std::set<Gate*>& gates)
        {
            std::vector<u32> res;
            res.reserve(gates.size());
            for (const auto* g : gates)
            {
                res.push_back(g->get_id());
            }
            std::sort(res.begin(), res.end());
            return res;
        }

        bool RegisterCandidate::operator==(const RegisterCandidate& rhs) const
        {
            return (this->m_size == rhs.m_size) && (this->m_in_reg_ids == rhs.m_in_reg_ids) && (this->m_out_reg_ids == rhs.m_out_reg_ids);
        }

        bool RegisterCandidate::operator<(const RegisterCandidate& rhs) const
        {
            // larger candidates come first, as the reduction of the candidates relies on a candidate being visited
            // before the smaller candidates that it may contain. The registers only break ties among candidates of
            // equal size and are compared by gate ID, so that the order does not depend on where the gates of the
            // netlist happen to be allocated. `m_is_round_based` is not part of the order because it is implied by the
            // two registers being equal.
            if (this->m_size != rhs.m_size)
            {
                return this->m_size > rhs.m_size;
            }
            if (this->m_in_reg_ids != rhs.m_in_reg_ids)
            {
                return this->m_in_reg_ids < rhs.m_in_reg_ids;
            }
            return this->m_out_reg_ids < rhs.m_out_reg_ids;
        }

        Netlist* RegisterCandidate::get_netlist() const
        {
            return m_netlist;
        }

        u32 RegisterCandidate::get_size() const
        {
            return m_size;
        }

        bool RegisterCandidate::is_round_based() const
        {
            return m_is_round_based;
        }

        const std::set<Gate*>& RegisterCandidate::get_input_reg() const
        {
            return m_in_reg;
        }

        const std::set<Gate*>& RegisterCandidate::get_output_reg() const
        {
            return m_out_reg;
        }
    }    // namespace hawkeye
}    // namespace hal