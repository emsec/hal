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
        for (const auto& [id, group] : state->gates_of_group)
        {
            Module* group_module = nl->create_module("group_" + std::to_string(id), nl->get_top_module());
            for (const auto& gateId : group)
            {
                group_module->assign_gate(nl->get_gate_by_id(gateId));
            }
        }
        return true;
    }
    std::set<std::set<Gate*>> state_to_module::create_sets(Netlist* nl, const std::shared_ptr<const Grouping>& state)
    {
        std::set<std::set<Gate*>> registers;
        log_info("dataflow", "found: {}", state->gates_of_group.size());
        for (const auto& [_, group] : state->gates_of_group)
        {
            std::set<Gate*> register_set;
            for (const auto& gateId : group)
            {
                register_set.insert(nl->get_gate_by_id(gateId));
            }
            registers.insert(register_set);
        }
        return registers;
    }
}    // namespace hal