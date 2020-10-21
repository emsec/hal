#include "hal_core/netlist/netlist_utils.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"

#include <queue>
#include <set>
namespace hal
{
    namespace netlist_utils
    {
        namespace
        {
            struct
            {
                std::unordered_map<u32, BooleanFunction> functions;
            } _cache;

            static BooleanFunction get_function_of_gate(const Gate* gate)
            {
                if (auto it = _cache.functions.find(gate->get_id()); it != _cache.functions.end())
                {
                    return it->second;
                }
                else
                {
                    BooleanFunction bf = gate->get_boolean_function();
                    for (auto const& input_pin : gate->get_input_pins())
                    {
                        const auto& input_net = gate->get_fan_in_net(input_pin);
                        bf                    = bf.substitute(input_pin, std::to_string(input_net->get_id()));
                    }
                    _cache.functions.emplace(gate->get_id(), bf);
                    return bf;
                }
            }
        }    // namespace

        BooleanFunction get_subgraph_function(const std::set<const Gate*> subgraph_gates, const Net* output_net)
        {
            /* check validity of subgraph_gates */
            if (subgraph_gates.empty())
            {
                log_error("verification", "parameter 'subgraph_gates' is empty");
                return BooleanFunction();
            }
            if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](auto& g) { return g == nullptr; }))
            {
                log_error("verification", "parameter 'subgraph_gates' contains a nullptr");
                return BooleanFunction();
            }
            if (output_net->get_num_of_sources() != 1)
            {
                log_error("verification", "target net has 0 or more than 1 sources.");
                return BooleanFunction();
            }
            auto nl         = (*subgraph_gates.begin())->get_netlist();
            auto start_gate = output_net->get_sources()[0]->get_gate();
            auto result     = get_function_of_gate(start_gate);

            std::queue<Net*> q;
            for (auto& n : start_gate->get_fan_in_nets())
            {
                q.push(n);
            }
            while (!q.empty())
            {
                auto n = q.front();
                q.pop();
                if (n->get_num_of_sources() != 1)
                {
                    log_error("verification", "net has 0 or more than 1 sources. not expanding the function here");
                    continue;
                }

                auto src_gate = n->get_sources()[0]->get_gate();

                // log_info("test", "processing gate {}", src_gate->get_name());
                if (subgraph_gates.find(src_gate) != subgraph_gates.end())
                {
                    result = result.substitute(std::to_string(n->get_id()), get_function_of_gate(src_gate));
                    //log_info("test", result.to_string());
                    for (auto& sn : src_gate->get_fan_in_nets())
                    {
                        q.push(sn);
                    }
                }
            }
            return result.optimize();
        }

        std::unique_ptr<Netlist> create_deepcopy(const Netlist* nl)
        {
            auto c_netlist = netlist_factory::create_netlist(nl->get_gate_library());

            // manager, netlist_id, and top_module are set in the constructor 

            // copy nets
            for (const auto& net : nl->get_nets()) {
                c_netlist->create_net(net->get_id(), net->get_name());
            }

            // copy gates
            for (const auto& gate : nl->get_gates()) {
                auto c_gate = c_netlist->create_gate(gate->get_id(), gate->get_type(), gate->get_name(), gate->get_location_x(), gate->get_location_y());
            
                for (const auto& in_point : gate->get_fan_in_endpoints()) {
                    const auto net_id = in_point->get_net()->get_id();
                    auto c_net = c_netlist->get_net_by_id(net_id);

                    c_net->add_destination(c_gate, in_point->get_pin());
                }

                for (const auto& out_point : gate->get_fan_out_endpoints()) {
                    const auto net_id = out_point->get_net()->get_id();
                    auto c_net = c_netlist->get_net_by_id(net_id);

                    c_net->add_source(c_gate, out_point->get_pin());
                }
            }

            // copy modules
            for (const auto& module : nl->get_modules()) {
                // ignore top module, since this is already created by the constructor
                if (module->get_id() == 1) {
                    continue;
                }
                std::vector<Gate*> c_gates;
                for (const auto& gate : module->get_gates()) {
                    // find gates of module in the copied netlist by id
                    auto c_gate = c_netlist->get_gate_by_id(gate->get_id());
                    c_gates.push_back(c_gate);
                }

                // create all modules with the top module as parent module and update later
                c_netlist->create_module(module->get_id(), module->get_name(), c_netlist->get_top_module(), c_gates);
            }

            // update parent_module in modules
            for (const auto& module : nl->get_modules()) {
                // ignore top_module
                if (module->get_parent_module() == nullptr) {
                    continue;
                }
                // find parent and child module in the copied netlist by id
                const auto module_id = module->get_id();
                const auto parent_id = module->get_parent_module()->get_id();
                auto c_module = c_netlist->get_module_by_id(module_id);
                auto c_parent = c_netlist->get_module_by_id(parent_id);

                c_module->set_parent_module(c_parent);
            }

            // copy grougpings
            for (const auto& grouping : nl->get_groupings()) {
                auto c_grouping = c_netlist->create_grouping(grouping->get_id(), grouping->get_name());
            
                for (const auto& module : grouping->get_modules()) {
                    const auto module_id = module->get_id();
                    c_grouping->assign_module_by_id(module_id);
                }

                for (const auto& net : grouping->get_nets()) {
                    const auto net_id = net->get_id();
                    c_grouping->assign_net_by_id(net_id);
                }

                for (const auto& gate : grouping->get_gates()) {
                    const auto gate_id = gate->get_id();
                    c_grouping->assign_net_by_id(gate_id);
                }
            }

            // mark globals
            for (const auto& global_input_net : nl->get_global_input_nets()) {
                auto c_global_input_net = c_netlist->get_net_by_id(global_input_net->get_id());
                c_netlist->mark_global_input_net(c_global_input_net);
            }
            for (const auto& global_output_net : nl->get_global_output_nets()) {
                auto c_global_output_net = c_netlist->get_net_by_id(global_output_net->get_id());
                c_netlist->mark_global_output_net(c_global_output_net);
            }
            for (const auto& gnd_gate : nl->get_gnd_gates()) {
                auto c_gnd_gate = c_netlist->get_gate_by_id(gnd_gate->get_id());
                c_netlist->mark_gnd_gate(gnd_gate);
            }
            for (const auto& vcc_gate : nl->get_vcc_gates()) {
                auto c_vcc_gate = c_netlist->get_gate_by_id(vcc_gate->get_id());
                c_netlist->mark_vcc_gate(vcc_gate);
            }

            c_netlist->set_design_name(nl->get_design_name());
            c_netlist->set_device_name(nl->get_device_name());
            c_netlist->set_input_filename(nl->get_input_filename());


            /* 
            * The ids for all components after the deepcopy are identical, however after the copy
            * the ids for newly created components might diverge.
            * This could be prevented by synching the id tracking members aswell, which would require additional getters/setters for those.
            */
            
            // update ids last, after all the creation
            // c_netlist->m_next_gate_id = nl->m_next_gate_id;
            // c_netlist->m_used_gate_ids = nl->m_used_gate_ids;
            // c_netlist->m_free_gate_ids = nl->m_free_gate_ids;

            // c_netlist->m_next_net_id = nl->m_next_net_id;
            // c_netlist->m_used_net_ids = nl->m_used_net_ids;
            // c_netlist->m_free_net_ids = nl->m_free_net_ids;

            // c_netlist->m_next_module_id = nl->m_next_module_id;
            // c_netlist->m_used_module_ids = nl->m_used_module_ids;
            // c_netlist->m_free_module_ids = nl->m_free_module_ids;

            // c_netlist->m_next_grouping_id = nl->m_next_grouping_id;
            // c_netlist->m_used_grouping_ids = nl->m_used_grouping_ids;
            // c_netlist->m_free_grouping_ids = nl->m_free_grouping_ids;


            return std::move(c_netlist);
        }
    }    // namespace netlist_utils

}    // namespace hal