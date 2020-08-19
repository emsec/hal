#include "processing/pass_collection.h"

#include "processing/passes/group_by_control_signals.h"
#include "processing/passes/group_by_input_output_size.h"
#include "processing/passes/group_by_successors_predecessors.h"
#include "processing/passes/group_by_successors_predecessors_iteratively.h"
#include "processing/passes/merge_successor_predecessor_groupings.h"
#include "processing/passes/split_by_successors_predecessors.h"

#include <algorithm>

namespace hal
{
    namespace processing
    {
        PassConfiguration::PassConfiguration(const pass_function& func)
        {
            static pass_id m_unique_id_it = 0;

            function = func;
            id       = ++m_unique_id_it;
        }

        namespace pass_collection
        {
            namespace
            {
                std::vector<PassConfiguration> m_all_passes;
                std::vector<PassConfiguration> m_intermediate_passes;
                std::unordered_map<pass_id, std::unordered_set<pass_id>> m_useless_follow_ups;
                bool m_initialized;

                PassConfiguration group_by_ctrl_sigs;

                PassConfiguration group_by_output_size;
                PassConfiguration group_by_input_size;

                PassConfiguration group_by_successors;
                PassConfiguration group_by_predecessors;

                PassConfiguration group_by_successors_iteratively;
                PassConfiguration group_by_predecessors_iteratively;

                PassConfiguration split_by_successors;
                PassConfiguration split_by_predecessors;

                void initialize()
                {
                    using namespace std::placeholders;

                    // start passes

                    group_by_ctrl_sigs = m_all_passes.emplace_back(std::bind(&group_by_control_signals::process, _1, true, true, true));

                    group_by_output_size = m_all_passes.emplace_back(std::bind(&group_by_input_output_size::process, _1, false));
                    group_by_input_size  = m_all_passes.emplace_back(std::bind(&group_by_input_output_size::process, _1, true));

                    group_by_successors   = m_all_passes.emplace_back(std::bind(&group_by_successors_predecessors::process, _1, true));
                    group_by_predecessors = m_all_passes.emplace_back(std::bind(&group_by_successors_predecessors::process, _1, false));

                    group_by_successors_iteratively   = m_all_passes.emplace_back(std::bind(&group_by_successors_predecessors_iteratively::process, _1, true));
                    group_by_predecessors_iteratively = m_all_passes.emplace_back(std::bind(&group_by_successors_predecessors_iteratively::process, _1, false));

                    split_by_successors   = m_all_passes.emplace_back(std::bind(&split_by_successors_predecessors::process, _1, true));
                    split_by_predecessors = m_all_passes.emplace_back(std::bind(&split_by_successors_predecessors::process, _1, false));

                    m_useless_follow_ups[group_by_successors.id].insert(split_by_successors.id);
                    m_useless_follow_ups[group_by_predecessors.id].insert(split_by_predecessors.id);
                    m_useless_follow_ups[group_by_successors_iteratively.id].insert(group_by_successors.id);
                    m_useless_follow_ups[group_by_predecessors_iteratively.id].insert(group_by_predecessors.id);

                    m_initialized = true;
                }

            }    // namespace

            std::vector<PassConfiguration> get_passes(const std::vector<std::vector<pass_id>>& previous_passes)
            {
                if (!m_initialized)
                {
                    initialize();
                }
                std::vector<pass_id> last_pass_ids;
                last_pass_ids.reserve(previous_passes.size());
                std::transform(previous_passes.begin(), previous_passes.end(), std::back_inserter(last_pass_ids), [](auto& vec) { return vec.back(); });

                std::vector<PassConfiguration> passes;

                for (auto& pass : m_all_passes)
                {
                    if (m_useless_follow_ups[pass.id].find(pass.id) == m_useless_follow_ups[pass.id].end())
                    {
                        passes.push_back(pass);
                    }
                }

                if (last_pass_ids.empty())
                {
                    if (auto it = std::find_if(passes.begin(), passes.end(), [](auto& p) { return p.id == split_by_successors.id; }); it != passes.end())
                    {
                        passes.erase(it);
                    }
                    if (auto it = std::find_if(passes.begin(), passes.end(), [](auto& p) { return p.id == split_by_predecessors.id; }); it != passes.end())
                    {
                        passes.erase(it);
                    }
                }

                return passes;
            }

        }    // namespace pass_collection
    }        // namespace processing
}    // namespace hal