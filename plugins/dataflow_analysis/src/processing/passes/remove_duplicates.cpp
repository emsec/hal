#include "dataflow_analysis/processing/passes/remove_duplicates.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/netlist/gate.h"

#include <list>
#include <map>

namespace hal
{
    namespace dataflow
    {
        namespace remove_duplicates
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool delete_from_smaller)
            {
                auto new_state = std::make_shared<Grouping>(state->netlist_abstr);

                std::map<u32, std::unordered_set<u32>> size_group_map;
                for (const auto& [group_id, gates] : state->gates_of_group)
                {
                    size_group_map[gates.size()].insert(group_id);
                }

                u32 id_counter = -1;
                std::unordered_set<u32> merged_gates;
                if (delete_from_smaller)
                {
                    /* iterate from smallest to largest group */
                    for (auto it = size_group_map.rbegin(); it != size_group_map.rend(); it++)
                    {
                        /* iterate through same sized groups */
                        for (const auto& group_id : it->second)
                        {
                            std::unordered_set<u32> new_group;

                            for (auto gate_id : state->gates_of_group.at(group_id))
                            {
                                if (merged_gates.find(gate_id) == merged_gates.end())
                                {
                                    new_group.insert(gate_id);
                                    merged_gates.insert(gate_id);
                                }
                            }

                            if (new_group.size() > 0)
                            {
                                u32 new_group_id = ++id_counter;

                                new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(*new_group.begin());
                                new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed.at(group_id);
                                new_state->gates_of_group[new_group_id].insert(new_group.begin(), new_group.end());
                                for (const auto& sg : new_group)
                                {
                                    new_state->parent_group_of_gate[sg] = new_group_id;
                                }
                            }
                        }
                    }
                }
                else
                {
                    /* remove dublicates from smallest groups */
                    for (auto it = size_group_map.begin(); it != size_group_map.end(); it++)
                    {
                        /* ignore single gates */
                        if (it->first == 1)
                        {
                            continue;
                        }

                        /* iterate through same sized groups */
                        for (const auto& group_id : it->second)
                        {
                            std::unordered_set<u32> new_group;

                            for (auto gate_id : state->gates_of_group.at(group_id))
                            {
                                if (merged_gates.find(gate_id) == merged_gates.end())
                                {
                                    new_group.insert(gate_id);
                                    merged_gates.insert(gate_id);
                                }
                            }

                            if (new_group.size() > 0)
                            {
                                u32 new_group_id = ++id_counter;

                                new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(*new_group.begin());
                                new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed[group_id];

                                new_state->gates_of_group[new_group_id].insert(new_group.begin(), new_group.end());
                                for (const auto& sg : new_group)
                                {
                                    new_state->parent_group_of_gate[sg] = new_group_id;
                                }
                            }
                        }
                    }

                    /* insert missing gates */
                    for (auto gate : state->netlist_abstr.all_sequential_gates)
                    {
                        u32 gate_id = gate->get_id();
                        if (merged_gates.find(gate_id) == merged_gates.end())
                        {
                            u32 new_group_id = ++id_counter;

                            new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(gate_id);
                            new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed[state->parent_group_of_gate[gate_id]];

                            new_state->gates_of_group[new_group_id].insert(gate_id);
                            new_state->parent_group_of_gate[gate_id] = new_group_id;
                        }
                    }
                }

                return new_state;
            }
        }    // namespace remove_duplicates
    }        // namespace dataflow
}