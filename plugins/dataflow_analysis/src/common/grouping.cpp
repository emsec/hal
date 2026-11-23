#include "dataflow_analysis/common/grouping.h"

#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <algorithm>

namespace hal
{
    namespace dataflow
    {
        Grouping::Grouping(const NetlistAbstraction& na) : netlist_abstr(na)
        {
        }

        Grouping::Grouping(const NetlistAbstraction& na, const std::vector<std::vector<Gate*>>& groups) : Grouping(na)
        {
            /* initialize state */
            i32 new_id_counter = -1;
            bool group_is_known;

            for (const auto* gate : na.target_gates)
            {
                group_is_known = false;
                for (const auto& gates : groups)
                {
                    for (const auto* g : gates)
                    {
                        if (gate == g)
                        {
                            group_is_known = true;
                            break;
                        }
                    }
                    if (group_is_known)
                    {
                        break;
                    }
                }
                if (!group_is_known)
                {
                    u32 new_group_id = (u32)(++new_id_counter);

                    this->group_control_fingerprint_map[new_group_id] = na.gate_to_fingerprint.at(gate->get_id());

                    this->gates_of_group[new_group_id].insert(gate->get_id());
                    this->parent_group_of_gate[gate->get_id()] = new_group_id;

                    this->operations_on_group_allowed[new_group_id] = true;
                }
            }

            /* merge groups known before execution */
            for (const auto& gates : groups)
            {
                if (gates.empty())
                {
                    continue;
                }

                u32 new_group_id = (u32)(++new_id_counter);

                this->operations_on_group_allowed[new_group_id]   = false;
                this->group_control_fingerprint_map[new_group_id] = na.gate_to_fingerprint.at(gates.front()->get_id());

                for (const auto& g : gates)
                {
                    auto gid = g->get_id();
                    this->gates_of_group[new_group_id].insert(gid);
                    this->parent_group_of_gate[gid] = new_group_id;
                }
            }
        }

        Grouping::Grouping(const Grouping& other) : Grouping(other.netlist_abstr)
        {
            group_control_fingerprint_map = other.group_control_fingerprint_map;
            gates_of_group                = other.gates_of_group;
            parent_group_of_gate          = other.parent_group_of_gate;
        }

        const std::set<std::set<u32>>& Grouping::get_comparison_data() const
        {
            // worker threads compare their results against the same groupings, so the cache is filled under the lock
            {
                std::shared_lock lock(cache.mutex);
                if (!cache.comparison_cache.empty())
                {
                    return cache.comparison_cache;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (cache.comparison_cache.empty())
            {
                for (const auto& it : gates_of_group)
                {
                    cache.comparison_cache.emplace(it.second.begin(), it.second.end());
                }
            }
            return cache.comparison_cache;
        }

        std::vector<u32> Grouping::get_group_ids() const
        {
            std::vector<u32> group_ids;
            group_ids.reserve(gates_of_group.size());
            for (const auto& [group_id, _] : gates_of_group)
            {
                group_ids.push_back(group_id);
            }
            std::sort(group_ids.begin(), group_ids.end());
            return group_ids;
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

        std::map<PinType, std::unordered_set<u32>> Grouping::get_control_signals_of_group(u32 group_id) const
        {
            std::map<PinType, std::unordered_set<u32>> res;

            for (auto gate : gates_of_group.at(group_id))
            {
                if (auto it = this->netlist_abstr.gate_to_control_signals.find(gate); it != this->netlist_abstr.gate_to_control_signals.end())
                {
                    res.insert(it->second.begin(), it->second.end());
                }
            }

            return res;
        }

        std::unordered_set<u32> Grouping::get_signals_of_group(u32 id, const std::unordered_map<u32, std::unordered_set<u32>>& signals) const
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

        std::set<u32> Grouping::get_register_stage_intersect_of_group(u32 id) const
        {
            static const std::unordered_set<u32> no_register_stages = {INT32_MAX};

            std::set<u32> intersect;
            bool first_gate = true;
            for (auto gate : this->gates_of_group.at(id))
            {
                const auto it       = this->netlist_abstr.gate_to_register_stages.find(gate);
                const auto& gate_rs = (it != this->netlist_abstr.gate_to_register_stages.end()) ? it->second : no_register_stages;

                if (first_gate)
                {
                    intersect.insert(gate_rs.begin(), gate_rs.end());
                    first_gate = false;
                }
                else
                {
                    for (auto stage_it = intersect.begin(); stage_it != intersect.end();)
                    {
                        stage_it = (gate_rs.find(*stage_it) != gate_rs.end()) ? std::next(stage_it) : intersect.erase(stage_it);
                    }
                }

                if (intersect.empty())
                {
                    break;
                }
            }

            return intersect;
        }

        std::unordered_set<u32> Grouping::get_successor_groups_of_group(u32 group_id) const
        {
            {
                std::shared_lock lock(cache.mutex);
                if (auto it = cache.suc_cache.find(group_id); it != cache.suc_cache.end())
                {
                    return it->second;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (auto it = cache.suc_cache.find(group_id); it != cache.suc_cache.end())
            {
                return it->second;
            }

            std::unordered_set<u32> successors;
            for (auto gate : gates_of_group.at(group_id))
            {
                for (auto gate_id : this->netlist_abstr.gate_to_successors.at(gate))
                {
                    successors.insert(parent_group_of_gate.at(gate_id));
                }
            }

            cache.suc_cache.emplace(group_id, successors);

            return successors;
        }

        std::unordered_set<u32> Grouping::get_predecessor_groups_of_group(u32 group_id) const
        {
            {
                std::shared_lock lock(cache.mutex);
                if (auto it = cache.pred_cache.find(group_id); it != cache.pred_cache.end())
                {
                    return it->second;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (auto it = cache.pred_cache.find(group_id); it != cache.pred_cache.end())
            {
                return it->second;
            }

            std::unordered_set<u32> predecessors;
            for (auto gate : gates_of_group.at(group_id))
            {
                for (auto gate_id : this->netlist_abstr.gate_to_predecessors.at(gate))
                {
                    predecessors.insert(parent_group_of_gate.at(gate_id));
                }
            }

            cache.pred_cache.emplace(group_id, predecessors);

            return predecessors;
        }

        std::unordered_set<u32> Grouping::get_known_successor_groups_of_group(u32 group_id) const
        {
            {
                std::shared_lock lock(cache.mutex);
                if (auto it = cache.suc_known_group_cache.find(group_id); it != cache.suc_known_group_cache.end())
                {
                    return it->second;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (auto it = cache.suc_known_group_cache.find(group_id); it != cache.suc_known_group_cache.end())
            {
                return it->second;
            }

            std::unordered_set<u32> successors;
            for (auto gate : gates_of_group.at(group_id))
            {
                for (auto known_group_id : this->netlist_abstr.gate_to_known_successor_groups.at(gate))
                {
                    successors.insert(known_group_id);
                }
            }

            cache.suc_known_group_cache.emplace(group_id, successors);

            return successors;
        }

        std::unordered_set<u32> Grouping::get_known_predecessor_groups_of_group(u32 group_id) const
        {
            {
                std::shared_lock lock(cache.mutex);
                if (auto it = cache.pred_known_group_cache.find(group_id); it != cache.pred_known_group_cache.end())
                {
                    return it->second;
                }
            }
            std::unique_lock lock(cache.mutex);

            // check again, since another thread might have gotten the unique lock first
            if (auto it = cache.pred_known_group_cache.find(group_id); it != cache.pred_known_group_cache.end())
            {
                return it->second;
            }

            std::unordered_set<u32> predecessors;
            for (auto gate : gates_of_group.at(group_id))
            {
                for (auto known_group_id : this->netlist_abstr.gate_to_known_predecessor_groups.at(gate))
                {
                    predecessors.insert(known_group_id);
                }
            }

            cache.pred_known_group_cache.emplace(group_id, predecessors);

            return predecessors;
        }

        bool Grouping::are_groups_allowed_to_merge(u32 group_1_id, u32 group_2_id, bool enforce_type_consistency) const
        {
            if (this->group_control_fingerprint_map.at(group_1_id) != this->group_control_fingerprint_map.at(group_2_id))
            {
                return false;
            }
            if (enforce_type_consistency
                && (this->netlist_abstr.nl->get_gate_by_id(*gates_of_group.at(group_1_id).begin())->get_type()
                    != this->netlist_abstr.nl->get_gate_by_id(*gates_of_group.at(group_2_id).begin())->get_type()))
            {
                return false;
            }
            if (!(this->operations_on_group_allowed.at(group_1_id) && this->operations_on_group_allowed.at(group_2_id)))
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

        bool Grouping::is_group_allowed_to_split(u32 group_id) const
        {
            if (this->operations_on_group_allowed.at(group_id))
            {
                return true;
            }
            return false;
        }
    }    // namespace dataflow
}    // namespace hal