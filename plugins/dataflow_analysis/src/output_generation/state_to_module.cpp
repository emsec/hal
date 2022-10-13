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
            // delete all modules that start with DANA
            for (auto const& mod : nl->get_modules())
            {
                if (mod->get_name().find("DANA") != std::string::npos)
                {
                    nl->delete_module(mod);
                }
            }
            log_info("dataflow", "succesufully deleted old DANA modules");

            // create new modules and try to keep hierachy if possible
            for (const auto& [id, group] : state->gates_of_group)
            {
                bool gate_hierachy_matches_for_all = true;
                bool first_run                     = true;
                hal::Module* reference_module      = nl->get_top_module();

                std::vector<Gate*> gates;
                for (const auto& gate_id : group)
                {
                    gates.push_back(nl->get_gate_by_id(gate_id));

                    if (first_run)
                    {
                        reference_module = nl->get_gate_by_id(gate_id)->get_module();
                        first_run        = false;
                    }
                    else if (!gate_hierachy_matches_for_all)
                    {
                        continue;
                    }
                    else if (nl->get_gate_by_id(gate_id)->get_module() != reference_module)
                    {
                        gate_hierachy_matches_for_all = false;
                    }
                }

                if (!gate_hierachy_matches_for_all)
                {
                    reference_module = nl->get_top_module();
                }
                nl->create_module("DANA_register_" + std::to_string(id), reference_module, gates);
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