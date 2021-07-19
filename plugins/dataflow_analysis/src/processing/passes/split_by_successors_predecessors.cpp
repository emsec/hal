#include "dataflow_analysis/processing/passes/split_by_successors_predecessors.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"

#include <map>
#include <set>

namespace hal
{
    namespace dataflow
    {
        namespace split_by_successors_predecessors
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool successors)
            {
                auto new_state = std::make_shared<Grouping>(state->netlist_abstr);

                u32 id_counter = -1;
                for (const auto& [group_id, gates] : state->gates_of_group)
                {
                    if (!state->is_group_allowed_to_split(group_id))
                    {
                        continue;
                    }

                    std::map<std::set<u32>, std::unordered_set<u32>> characteristics_map;
                    for (auto gate : gates)
                    {
                        std::set<u32> characteristics_of_gate;
                        if (successors)
                        {
                            for (auto gate_successors : state->netlist_abstr.gate_to_successors.at(gate))
                            {
                                characteristics_of_gate.insert(state->parent_group_of_gate.at(gate_successors));
                            }
                        }
                        else
                        {
                            for (auto gate_predecessors : state->netlist_abstr.gate_to_predecessors.at(gate))
                            {
                                characteristics_of_gate.insert(state->parent_group_of_gate.at(gate_predecessors));
                            }
                        }
                        characteristics_map[characteristics_of_gate].insert(gate);
                    }

                    /* merge gates */
                    for (auto gates_to_merge : characteristics_map)
                    {
                        u32 new_group_id = ++id_counter;

                        new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(*gates_to_merge.second.begin());
                        new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed.at(group_id);
                        new_state->gates_of_group[new_group_id].insert(gates_to_merge.second.begin(), gates_to_merge.second.end());
                        for (const auto& sg : gates_to_merge.second)
                        {
                            new_state->parent_group_of_gate[sg] = new_group_id;
                        }
                    }
                }

                return new_state;
            }

        }    // namespace split_by_successors_predecessors
    }        // namespace dataflow
}