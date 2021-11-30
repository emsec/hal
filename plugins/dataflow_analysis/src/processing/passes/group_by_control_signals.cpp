#include "dataflow_analysis/processing/passes/group_by_control_signals.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/utilities/log.h"

#include <list>

namespace hal
{
    namespace dataflow
    {
        namespace group_by_control_signals
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool clock, bool clock_enable, bool reset, bool set)
            {
                auto new_state = std::make_shared<Grouping>(state->netlist_abstr);

                /* check characteristics */
                std::map<std::set<u32>, std::unordered_set<u32>> characteristics_map;
                for (const auto& [group_id, gates] : state->gates_of_group)
                {
                    std::set<u32> candidate_characteristic_set;
                    if (clock)
                    {
                        auto signals = state->get_clock_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }
                    if (clock_enable)
                    {
                        auto signals = state->get_control_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }
                    if (reset)
                    {
                        auto signals = state->get_reset_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }
                    if (set)
                    {
                        auto signals = state->get_set_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }

                    characteristics_map[candidate_characteristic_set].insert(group_id);
                }

                /* check if merge is allowed */
                std::vector<std::vector<u32>> merge_sets;
                for (auto& merge_candidates : characteristics_map)
                {
                    auto& work_list = merge_candidates.second;
                    while (!work_list.empty())
                    {
                        auto it       = work_list.begin();
                        auto group_id = *it;
                        it            = work_list.erase(it);

                        std::vector<u32> merge_set = {group_id};

                        while (it != work_list.end())
                        {
                            auto test_group_id = *it;

                            if (!state->are_groups_allowed_to_merge(group_id, test_group_id))
                            {
                                ++it;
                                continue;
                            }

                            merge_set.push_back(test_group_id);
                            it = work_list.erase(it);
                        }
                        merge_sets.push_back(merge_set);
                    }
                }

                /* merge groups */
                u32 id_counter = -1;
                for (const auto& groups_to_merge : merge_sets)
                {
                    u32 new_group_id = ++id_counter;

                    for (const auto& old_group : groups_to_merge)
                    {
                        auto gates                                             = state->gates_of_group.at(old_group);
                        new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(*gates.begin());
                        new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed.at(old_group);
                        new_state->gates_of_group[new_group_id].insert(gates.begin(), gates.end());
                        for (const auto& sg : gates)
                        {
                            new_state->parent_group_of_gate[sg] = new_group_id;
                        }
                    }
                }

                return new_state;
            }

            std::shared_ptr<Grouping> pure_control_signals_process(const std::shared_ptr<Grouping>& state, bool clock, bool clock_enable, bool reset, bool set)
            {
                auto new_state = std::make_shared<Grouping>(state->netlist_abstr);

                /* check characteristics */
                std::map<std::set<u32>, std::unordered_set<u32>> characteristics_map;
                for (const auto& [group_id, gates] : state->gates_of_group)
                {
                    std::set<u32> candidate_characteristic_set;
                    if (clock)
                    {
                        auto signals = state->get_clock_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }
                    if (clock_enable)
                    {
                        auto signals = state->get_control_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }
                    if (reset)
                    {
                        auto signals = state->get_reset_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }
                    if (set)
                    {
                        auto signals = state->get_set_signals_of_group(group_id);
                        candidate_characteristic_set.insert(signals.begin(), signals.end());
                    }

                    characteristics_map[candidate_characteristic_set].insert(group_id);
                }

                /* merge groups */
                u32 id_counter = -1;
                for (const auto& it : characteristics_map)
                {
                    auto& groups_to_merge = it.second;
                    u32 new_group_id      = ++id_counter;

                    for (const auto& old_group : groups_to_merge)
                    {
                        auto gates                                             = state->gates_of_group.at(old_group);
                        new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(*gates.begin());
                        new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed.at(old_group);
                        new_state->gates_of_group[new_group_id].insert(gates.begin(), gates.end());
                        for (const auto& g : gates)
                        {
                            new_state->parent_group_of_gate[g] = new_group_id;
                        }
                    }
                }

                return new_state;
            }

        }    // namespace group_by_control_signals
    }        // namespace dataflow
}