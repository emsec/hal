#include "dataflow_analysis/output_generation/state_to_module.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    bool state_to_module::create_modules(Netlist* nl, const std::shared_ptr<const Grouping>& state)
    {
        // auto groups = state->candidate_set;

        // for (const auto& group : groups)
        // {
        //     module* group_module = nl->create_module("group_" + std::to_string(group->id), nl->get_top_module());
        //     for (const auto& subgroup : group->m_subgroups)
        //     {
        //         module* subgroup_module = nl->create_module("subgroup_" + std::to_string(subgroup->id), group_module);
        //         for (const auto& ff : subgroup->ffs)
        //         {
        //             subgroup_module->assign_gate(ff);
        //         }
        //     }
        // }

        return true;
    }

    std::set<std::set<Gate*>> state_to_module::create_sets(Netlist* nl, const std::shared_ptr<const Grouping>& state)
    {
        // std::set<std::set<Gate*>> registers;
        // auto groups = state->candidate_set;

        // log_info("dataflow", "found: {}", groups.size());

        // for (const auto& group : groups)
        // {
        //     std::set<Gate*> register_set;
        //     for (const auto& subgroup : group->m_subgroups)
        //     {
        //         for (const auto& ff : subgroup->ffs)
        //         {
        //             register_set.insert(ff);
        //         }
        //     }
        //     registers.insert(register_set);
        // }

        // return registers;

        return std::set<std::set<Gate*>>();
    }
}    // namespace hal
