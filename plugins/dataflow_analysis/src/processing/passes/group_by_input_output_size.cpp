#include "dataflow_analysis/processing/passes/group_by_input_output_size.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"

#include <list>
#include <map>
#include <set>

namespace hal
{
    namespace dataflow
    {
        namespace group_by_input_output_size
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool successors)
            {
                auto new_state = std::make_shared<Grouping>(state->netlist_abstr);

                /* check characteristics */
                std::map<std::pair<u32, u32>, std::list<u32>> characteristics_map;
                for (const auto& [group_id, gates] : state->gates_of_group)
                {
                    std::pair<u32, u32> min_max_connections;
                    bool first = true;
                    for (auto g : gates)
                    {
                        u32 val;
                        if (successors)
                        {
                            val = state->netlist_abstr.gate_to_successors.at(g).size();
                        }
                        else
                        {
                            val = state->netlist_abstr.gate_to_predecessors.at(g).size();
                        }

                        if (first)
                        {
                            min_max_connections.first  = val;
                            min_max_connections.second = val;
                        }
                        else
                        {
                            min_max_connections.first  = std::min(min_max_connections.first, val);
                            min_max_connections.second = std::max(min_max_connections.second, val);
                        }
                        first = false;
                    }

                    characteristics_map[min_max_connections].push_back(group_id);
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
                        new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed.at(old_group);;
                        new_state->gates_of_group[new_group_id].insert(gates.begin(), gates.end());
                        for (const auto& sg : gates)
                        {
                            new_state->parent_group_of_gate[sg] = new_group_id;
                        }
                    }
                }

                return new_state;
            }

        }    // namespace group_by_input_output_size
    }        // namespace dataflow
}