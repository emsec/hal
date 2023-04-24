#include "dataflow_analysis/output_generation/result.h"

#include "hal_core/netlist/netlist.h"

namespace hal
{
    namespace dataflow
    {
        dataflow::Result::Result(Netlist* nl, const Grouping& grouping)
        {
            m_netlist = nl;

            for (const auto& [group_id, gate_ids] : grouping.gates_of_group)
            {
                std::unordered_set<Gate*> gates;
                for (const auto gate_id : gate_ids)
                {
                    auto* gate = m_netlist->get_gate_by_id(gate_id);
                    gates.insert(gate);
                    m_parent_group_of_gate[gate] = group_id;
                }
                m_gates_of_group[group_id] = gates;

                for (const auto net_id : grouping.get_clock_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::clock].insert(m_netlist->get_net_by_id(net_id));
                }

                for (const auto net_id : grouping.get_control_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::enable].insert(m_netlist->get_net_by_id(net_id));
                }

                for (const auto net_id : grouping.get_reset_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::reset].insert(m_netlist->get_net_by_id(net_id));
                }

                for (const auto net_id : grouping.get_set_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::set].insert(m_netlist->get_net_by_id(net_id));
                }

                m_group_successors[group_id]   = grouping.get_successor_groups_of_group(group_id);
                m_group_predecessors[group_id] = grouping.get_predecessor_groups_of_group(group_id);
            }
        }

        Netlist* dataflow::Result::get_netlist() const
        {
            return m_netlist;
        }

        const std::unordered_map<u32, std::unordered_set<Gate*>>& dataflow::Result::get_groups() const
        {
            return m_gates_of_group;
        }

        hal::Result<std::unordered_set<Gate*>> dataflow::Result::get_gates_of_group(const u32 group_id) const
        {
            if (const auto it = m_gates_of_group.find(group_id); it != m_gates_of_group.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }

        hal::Result<u32> dataflow::Result::get_group_id_of_gate(const Gate* gate) const
        {
            if (!gate)
            {
                return ERR("gate is a nullptr.");
            }

            if (const auto it = m_parent_group_of_gate.find(gate); it != m_parent_group_of_gate.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("gate is not part of a group.");
            }
        }

        hal::Result<std::unordered_set<Net*>> dataflow::Result::get_control_nets_of_group(const u32 group_id, const PinType type) const
        {
            if (const auto group_it = m_group_signals.find(group_id); group_it != m_group_signals.end())
            {
                if (const auto type_it = group_it->second.find(type); type_it != group_it->second.end())
                {
                    return OK(type_it->second);
                }
                else
                {
                    return OK({});
                }
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }

        hal::Result<std::unordered_set<u32>> dataflow::Result::get_group_successors(const u32 group_id) const
        {
            if (const auto it = m_group_successors.find(group_id); it != m_group_successors.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }

        hal::Result<std::unordered_set<u32>> dataflow::Result::get_group_predecessors(const u32 group_id) const
        {
            if (const auto it = m_group_predecessors.find(group_id); it != m_group_predecessors.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }
    }    // namespace dataflow
}    // namespace hal