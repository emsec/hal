#include "dataflow_analysis/common/netlist_abstraction.h"

#include "dataflow_analysis/common/grouping.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace dataflow
    {
        NetlistAbstraction::NetlistAbstraction(Netlist* nl_arg) : nl(nl_arg)
        {
        }

        std::shared_ptr<Grouping> NetlistAbstraction::create_initial_grouping(std::vector<std::vector<u32>> known_groups) const
        {
            /* create state */
            auto new_state = std::make_shared<Grouping>(*this);

            /* initialize state */
            u32 new_id_counter = -1;
            bool group_is_known;

            for (const auto& gate : new_state->netlist_abstr.all_sequential_gates)
            {
                group_is_known = false;
                for (const auto& known_group : known_groups)
                {
                    for (const auto& gate_id : known_group)
                    {
                        if (gate->get_id() == gate_id)
                        {
                            group_is_known = true;
                            break;
                        }
                    }
                    if (group_is_known)
                        break;
                }
                if (!group_is_known)
                {
                    u32 new_group_id = ++new_id_counter;

                    new_state->group_control_fingerprint_map[new_group_id] = this->gate_to_fingerprint.at(gate->get_id());

                    new_state->gates_of_group[new_group_id].insert(gate->get_id());
                    new_state->parent_group_of_gate[gate->get_id()] = new_group_id;

                    new_state->operations_on_group_allowed[new_group_id] = true;
                }
            }

            /* merge groups known before execution */
            for (const auto& gates : known_groups)
            {
                if (gates.empty())
                {
                    continue;
                }
                
                u32 new_group_id = ++new_id_counter;

                new_state->operations_on_group_allowed[new_group_id] = false;
                new_state->group_control_fingerprint_map[new_group_id] = this->gate_to_fingerprint.at(*gates.begin());
                new_state->gates_of_group[new_group_id].insert(gates.begin(), gates.end());

                for (const auto& gate_id : gates)
                {
                    new_state->parent_group_of_gate[gate_id] = new_group_id;
                }
            }

            return new_state;
        }
    }    // namespace dataflow
}    // namespace hal
