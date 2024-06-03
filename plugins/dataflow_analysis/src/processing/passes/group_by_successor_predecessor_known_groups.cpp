#include "dataflow_analysis/processing/passes/group_by_successor_predecessor_known_groups.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/processing/configuration.h"

#include <list>
#include <map>
#include <set>

namespace hal
{
    namespace dataflow
    {
        namespace group_by_successor_predecessor_known_groups
        {
            std::shared_ptr<Grouping> process(const processing::Configuration& config, const std::shared_ptr<Grouping>& state, bool successors)
            {
                auto new_state = std::make_shared<Grouping>(state->netlist_abstr);

                /* check characteristics */
                std::map<std::set<u32>, std::list<u32>> characteristics_map;
                std::vector<u32> groups_with_no_characteristics;
                for (const auto& [group_id, group] : state->gates_of_group)
                {
                    std::set<u32> characteristics_of_group;

                    if (successors)
                    {
                        auto successing_known_group = state->get_known_successor_groups_of_group(group_id);
                        characteristics_of_group.insert(successing_known_group.begin(), successing_known_group.end());
                    }
                    else
                    {
                        auto predecessing_known_group = state->get_known_predecessor_groups_of_group(group_id);
                        characteristics_of_group.insert(predecessing_known_group.begin(), predecessing_known_group.end());
                    }

                    if (characteristics_of_group.empty())
                    {
                        groups_with_no_characteristics.push_back(group_id);
                    }
                    else
                    {
                        characteristics_map[characteristics_of_group].push_back(group_id);
                    }
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

                            if (!state->are_groups_allowed_to_merge(group_id, test_group_id, config.enforce_type_consistency))
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

                /* add all groups with no characteristics as singleton merge set */
                for (const auto& group_id : groups_with_no_characteristics)
                {
                    merge_sets.push_back({group_id});
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

        }    // namespace group_by_successor_predecessor_known_groups
    }    // namespace dataflow
}    // namespace hal