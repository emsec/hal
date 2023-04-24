#include "dataflow_analysis/processing/passes/merge_states.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/processing/passes/remove_duplicates.h"

namespace hal
{
    namespace dataflow
    {
        namespace merge_states
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<const Grouping>& state1, const std::shared_ptr<const Grouping>& state2, bool delete_from_smaller)
            {
                auto new_state = std::make_shared<Grouping>(state1->netlist_abstr);

                u32 id_counter = -1;

                for (const auto& state : {state1, state2})
                {
                    for (const auto& [group_id, gates] : state->gates_of_group)
                    {
                        u32 new_group_id = ++id_counter;

                        new_state->group_control_fingerprint_map[new_group_id] = new_state->netlist_abstr.gate_to_fingerprint.at(*gates.begin());
                        new_state->operations_on_group_allowed[new_group_id]   = state->operations_on_group_allowed.at(group_id);

                        new_state->gates_of_group[new_group_id].insert(gates.begin(), gates.end());
                        for (const auto& sg : gates)
                        {
                            new_state->parent_group_of_gate[sg] = new_group_id;
                        }
                    }
                }

                new_state = remove_duplicates::process(new_state, delete_from_smaller);

                return new_state;
            }    // namespace merge_states

        }    // namespace merge_states
    }        // namespace dataflow
}