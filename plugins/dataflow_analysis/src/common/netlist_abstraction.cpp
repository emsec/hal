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

        std::shared_ptr<Grouping> NetlistAbstraction::create_initial_grouping() const
        {
            /* create state */
            auto new_state = std::make_shared<Grouping>(*this);

            /* initialize state */
            u32 new_id_counter = -1;

            for (auto gate : new_state->netlist_abstr.all_sequential_gates)
            {
                u32 new_group_id = ++new_id_counter;

                new_state->group_control_fingerprint_map[new_group_id] = this->gate_to_fingerprint.at(gate->get_id());

                new_state->gates_of_group[new_group_id].insert(gate->get_id());
                new_state->parent_group_of_gate[gate->get_id()] = new_group_id;
            }

            return new_state;
        }
    }    // namespace dataflow
}    // namespace hal
