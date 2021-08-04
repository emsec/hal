#include "dataflow_analysis/common/grouping.h"

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace dataflow
    {
        Grouping::Grouping(const NetlistAbstraction& na) : netlist_abstr(na)
        {
        }

        Grouping::Grouping(const Grouping& other) : Grouping(other.netlist_abstr)
        {
            group_control_fingerprint_map = other.group_control_fingerprint_map;
            gates_of_group                = other.gates_of_group;
            parent_group_of_gate          = other.parent_group_of_gate;
        }

        const std::set<std::set<u32>>& Grouping::get_comparison_data() const
        {
            if (cache.comparison_cache.empty())
            {
                for (const auto& it : gates_of_group)
                {
                    cache.comparison_cache.emplace(it.second.begin(), it.second.end());
                }
            }
            return cache.comparison_cache;
        }

        bool Grouping::operator==(const Grouping& other) const
        {
            if (gates_of_group.size() != other.gates_of_group.size())
            {
                return false;
            }

            return get_comparison_data() == other.get_comparison_data();
        }

        bool Grouping::operator!=(const Grouping& other) const
        {
            return !(*this == other);
        }

        std::unordered_set<u32> Grouping::get_clock_signals_of_group(u32 id)
        {
            return get_signals_of_group(id, netlist_abstr.gate_to_clock_signals);
        }

        std::unordered_set<u32> Grouping::get_control_signals_of_group(u32 id)
        {
            return get_signals_of_group(id, netlist_abstr.gate_to_enable_signals);
        }

        std::unordered_set<u32> Grouping::get_reset_signals_of_group(u32 id)
        {
            return get_signals_of_group(id, netlist_abstr.gate_to_reset_signals);
        }

        std::unordered_set<u32> Grouping::get_set_signals_of_group(u32 id)
        {
            return get_signals_of_group(id, netlist_abstr.gate_to_set_signals);
        }

        std::unordered_set<u32> Grouping::get_signals_of_group(u32 id, const std::unordered_map<u32, std::unordered_set<u32>>& signals)
        {
            std::unordered_set<u32> res;

            for (auto gate : gates_of_group.at(id))
            {
                if (auto it = signals.find(gate); it != signals.end())
                {
                    res.insert(it->second.begin(), it->second.end());
                }
            }

            return res;
        }

        std::set<u32> Grouping::get_register_stage_intersect_of_group(u32 id)
        {
            std::vector<u32> intersect;
            for (auto gate : this->gates_of_group.at(id))
            {
                // check if gate has register_stages
                auto it = netlist_abstr.gate_to_register_stages.find(gate);
                if (it != netlist_abstr.gate_to_register_stages.end())
                {
                    auto gate_rs = std::set<u32>(it->second.begin(), it->second.end());
                    if (intersect.empty())
                    {
                        intersect.insert(intersect.end(), gate_rs.begin(), gate_rs.end());
                    }
                    else
                    {
                        auto end_it = std::set_intersection(intersect.begin(), intersect.end(), gate_rs.begin(), gate_rs.end(), intersect.begin());
                        intersect.erase(end_it, intersect.end());
                    }

                    if (intersect.empty())
                    {
                        break;
                    }
                }
                else
                {
                    intersect.push_back(INT32_MAX);
                }
            }

            return std::set<u32>(intersect.begin(), intersect.end());
        }

        std::unordered_set<u32> Grouping::get_successor_groups_of_group(u32 id)
        {
            {
                std::shared_lock lock(cache.mutex);
                if (auto it = cache.suc_cache.find(id); it != cache.suc_cache.end())
                {
                    return it->second;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (auto it = cache.suc_cache.find(id); it != cache.suc_cache.end())
            {
                return it->second;
            }

            std::unordered_set<u32> successors;
            for (auto gate : gates_of_group.at(id))
            {
                for (auto gate_id : netlist_abstr.gate_to_successors.at(gate))
                {
                    successors.insert(parent_group_of_gate.at(gate_id));
                }
            }

            cache.suc_cache.emplace(id, successors);

            return successors;
        }

        std::unordered_set<u32> Grouping::get_predecessor_groups_of_group(u32 id)
        {
            {
                std::shared_lock lock(cache.mutex);
                if (auto it = cache.pred_cache.find(id); it != cache.pred_cache.end())
                {
                    return it->second;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (auto it = cache.pred_cache.find(id); it != cache.pred_cache.end())
            {
                return it->second;
            }

            std::unordered_set<u32> predecessors;
            for (auto gate : gates_of_group.at(id))
            {
                for (auto gate_id : netlist_abstr.gate_to_predecessors.at(gate))
                {
                    predecessors.insert(parent_group_of_gate.at(gate_id));
                }
            }

            cache.pred_cache.emplace(id, predecessors);

            return predecessors;
        }

        bool Grouping::are_groups_allowed_to_merge(u32 group_1_id, u32 group_2_id)
        {
            if (this->group_control_fingerprint_map.at(group_1_id) != this->group_control_fingerprint_map.at(group_2_id))
            {
                return false;
            }
            /* without type check
    if (netlist_abstr.nl->get_gate_by_id(*gates_of_group.at(group_1_id).begin())->get_type() != netlist_abstr.nl->get_gate_by_id(*gates_of_group.at(group_2_id).begin())->get_type())
    {
        return false;
    }
     */
            if (!(this->operations_on_group_allowed[group_1_id] && this->operations_on_group_allowed[group_2_id]))
            {
                return false;
            }

            bool merged_allowed_register_stage = false;

            auto register_stages_group_2 = this->get_register_stage_intersect_of_group(group_2_id);
            for (auto stage : this->get_register_stage_intersect_of_group(group_1_id))
            {
                if (register_stages_group_2.find(stage) != register_stages_group_2.end())
                {
                    merged_allowed_register_stage = true;
                    break;
                }
            }
            return merged_allowed_register_stage;
        }

        bool Grouping::is_group_allowed_to_split(u32 group_id)
        {
            if (this->operations_on_group_allowed[group_id])
            {
                return true;
            }
            return false;
        }
    }    // namespace dataflow
}    // namespace hal