#include "dataflow_analysis/output_generation/state_to_module.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <vector>
namespace hal
{
    namespace dataflow
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
        std::vector<std::vector<Gate*>> state_to_module::create_sets(Netlist* nl, const std::shared_ptr<const Grouping>& state)
        {
            std::vector<std::vector<Gate*>> registers;
            for (const auto& [_, group] : state->gates_of_group)
            {
                std::vector<Gate*> register_vector;
                for (const auto& gateId : group)
                {
                    register_vector.push_back(nl->get_gate_by_id(gateId));
                }
                registers.push_back(register_vector);
            }
            return registers;
        }
    }    // namespace dataflow
}    // namespace hal