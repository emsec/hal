#include "output_generation/state_to_module.h"

#include "common/grouping.h"
#include "common/netlist_abstraction.h"
#include "core/log.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

namespace hal
{
    bool state_to_module::create_modules(const std::shared_ptr<Netlist>& nl, const std::shared_ptr<const Grouping>& state)
    {
        // auto groups = state->candidate_set;

        // for (const auto& group : groups)
        // {
        //     std::shared_ptr<module> group_module = nl->create_module("group_" + std::to_string(group->id), nl->get_top_module());
        //     for (const auto& subgroup : group->m_subgroups)
        //     {
        //         std::shared_ptr<module> subgroup_module = nl->create_module("subgroup_" + std::to_string(subgroup->id), group_module);
        //         for (const auto& ff : subgroup->ffs)
        //         {
        //             subgroup_module->assign_gate(ff);
        //         }
        //     }
        // }

        return true;
    }

    std::set<std::set<std::shared_ptr<Gate>>> state_to_module::create_sets(const std::shared_ptr<Netlist>& nl, const std::shared_ptr<const Grouping>& state)
    {
        // std::set<std::set<std::shared_ptr<Gate>>> registers;
        // auto groups = state->candidate_set;

        // log_info("dataflow", "found: {}", groups.size());

        // for (const auto& group : groups)
        // {
        //     std::set<std::shared_ptr<Gate>> register_set;
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

        return std::set<std::set<std::shared_ptr<Gate>>>();
    }
}    // namespace hal