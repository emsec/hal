#include "dataflow_analysis/processing/passes/group_by_successors_predecessors_iteratively.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/processing/passes/group_by_successors_predecessors.h"

namespace hal
{
    namespace dataflow
    {
        namespace group_by_successors_predecessors_iteratively
        {
            std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool successors)
            {
                auto new_state  = group_by_successors_predecessors::process(state, successors);
                auto last_state = new_state;

                u32 last_candidate_set_size = state->gates_of_group.size();
                while (new_state->gates_of_group.size() < last_candidate_set_size)
                {
                    last_candidate_set_size = new_state->gates_of_group.size();
                    last_state              = new_state;
                    new_state               = group_by_successors_predecessors::process(new_state, successors);
                }

                return last_state;
            }

        }    // namespace group_by_successors_predecessors_iteratively
    }        // namespace dataflow
}