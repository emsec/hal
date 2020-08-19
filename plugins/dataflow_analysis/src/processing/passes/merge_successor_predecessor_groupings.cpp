#include "processing/passes/merge_successor_predecessor_groupings.h"

#include "common/netlist_abstraction.h"
#include "processing/passes/group_by_successors_predecessors.h"
#include "processing/passes/merge_states.h"

namespace hal
{
    namespace merge_successor_predecessor_groupings
    {
        std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool delete_from_smaller)
        {
            auto new_state1 = group_by_successors_predecessors::process(state, true);
            auto new_state2 = group_by_successors_predecessors::process(state, false);

            auto merged_state = merge_states::process(new_state1, new_state2, delete_from_smaller);

            return merged_state;
        }

    }    // namespace merge_successor_predecessor_groupings
}    // namespace hal