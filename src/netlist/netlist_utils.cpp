#include "hal_core/netlist/netlist_utils.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"

#include <queue>
#include <unordered_set>

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

            static BooleanFunction get_function_of_gate(const Gate* const gate)
            {
                if (auto it = _cache.functions.find(gate->get_id()); it != _cache.functions.end())
                {
                    return it->second;
                }
                else
                {
                    BooleanFunction bf = gate->get_boolean_function();

                    for (const std::string& input_pin : gate->get_input_pins())
                    {
                        const Net* const input_net = gate->get_fan_in_net(input_pin);
                        if (input_net == nullptr)
                        {
                            // if no net is connected, the input pin name cannot be replaced
                            log_warning("netlist", "not net is connected to input pin '{}' of gate with ID {}, cannot replace pin name with net ID.", input_pin, gate->get_id());
                            return bf;
                        }

                        bf = bf.substitute(input_pin, std::to_string(input_net->get_id()));
                    }
                    _cache.functions.emplace(gate->get_id(), bf);
                    return bf;
                }
            }
        }    // namespace

        BooleanFunction get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* output_net)
        {
            /* check validity of subgraph_gates */
            if (subgraph_gates.empty())
            {
                log_error("netlist", "no gates given to determine the Boolean function of.");
                return BooleanFunction();
            }
            else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
            {
                log_error("netlist", "set of gates contains a nullptr.");
                return BooleanFunction();
            }
            else if (output_net == nullptr)
            {
                log_error("netlist", "nullptr given for target net.");
                return BooleanFunction();
            }
            else if (output_net->get_num_of_sources() > 1)
            {
                log_error("netlist", "target net with ID {} has more than one source.", output_net->get_id());
                return BooleanFunction();
            }
            else if (output_net->get_num_of_sources() == 0)
            {
                log_error("netlist", "target net with ID {} has no sources.", output_net->get_id());
                return BooleanFunction();
            }

            const Gate* start_gate = output_net->get_sources()[0]->get_gate();
            BooleanFunction result = get_function_of_gate(start_gate);

            std::queue<const Net*> q;
            for (const Net* n : start_gate->get_fan_in_nets())
            {
                q.push(n);
            }

            std::unordered_set<const Net*> visited_nets;

            while (!q.empty())
            {
                const Net* n = q.front();
                q.pop();

                if (visited_nets.find(n) != visited_nets.end())
                {
                    log_error("netlist", "detected infinite loop at net with ID {}, cannot determine Boolean function of the subgraph.", n->get_id());
                    return BooleanFunction();
                }
                visited_nets.insert(n);

                if (n->get_num_of_sources() > 1)
                {
                    log_error("netlist", "net with ID {} has more than one source, cannot expand Boolean function in this direction.", n->get_id());
                    continue;
                }
                else if (n->get_num_of_sources() == 0)
                {
                    continue;
                }

                const Gate* src_gate = n->get_sources()[0]->get_gate();

                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) != subgraph_gates.end())
                {
                    result = result.substitute(std::to_string(n->get_id()), get_function_of_gate(src_gate));

                    for (const Net* sn : src_gate->get_fan_in_nets())
                    {
                        q.push(sn);
                    }
                }
            }

            return result;
        }

        std::unique_ptr<Netlist> copy_netlist(const Netlist* nl)
        {
            std::unique_ptr<Netlist> c_netlist = netlist_factory::create_netlist(nl->get_gate_library());

            // manager, netlist_id, and top_module are set in the constructor

            // copy nets
            for (const Net* net : nl->get_nets())
            {
                c_netlist->create_net(net->get_id(), net->get_name());
            }

            // copy gates
            for (const Gate* gate : nl->get_gates())
            {
                Gate* c_gate = c_netlist->create_gate(gate->get_id(), gate->get_type(), gate->get_name(), gate->get_location_x(), gate->get_location_y());

                for (const auto& [name, func] : gate->get_boolean_functions(true))
                {
                    c_gate->add_boolean_function(name, func);
                }

                for (const Endpoint* in_point : gate->get_fan_in_endpoints())
                {
                    const auto net_id = in_point->get_net()->get_id();
                    auto c_net        = c_netlist->get_net_by_id(net_id);

                    c_net->add_destination(c_gate, in_point->get_pin());
                }

                for (const Endpoint* out_point : gate->get_fan_out_endpoints())
                {
                    const auto net_id = out_point->get_net()->get_id();
                    auto c_net        = c_netlist->get_net_by_id(net_id);

                    c_net->add_source(c_gate, out_point->get_pin());
                }
            }

            // copy modules
            for (const Module* module : nl->get_modules())
            {
                // ignore top module, since this is already created by the constructor
                if (module->get_id() == 1)
                {
                    continue;
                }

                std::vector<Gate*> c_gates;
                for (const Gate* gate : module->get_gates())
                {
                    // find gates of module in the copied netlist by id
                    Gate* c_gate = c_netlist->get_gate_by_id(gate->get_id());
                    c_gates.push_back(c_gate);
                }

                // create all modules with the top module as parent module and update later
                c_netlist->create_module(module->get_id(), module->get_name(), c_netlist->get_top_module(), c_gates);
            }

            // update parent_module in modules
            for (const Module* module : nl->get_modules())
            {
                // ignore top_module
                if (module->get_parent_module() == nullptr)
                {
                    continue;
                }

                // find parent and child module in the copied netlist by id
                const u32 module_id = module->get_id();
                const u32 parent_id = module->get_parent_module()->get_id();
                Module* c_module    = c_netlist->get_module_by_id(module_id);
                Module* c_parent    = c_netlist->get_module_by_id(parent_id);

                c_module->set_parent_module(c_parent);
            }

            // copy grougpings
            for (const Grouping* grouping : nl->get_groupings())
            {
                Grouping* c_grouping = c_netlist->create_grouping(grouping->get_id(), grouping->get_name());

                for (const Module* module : grouping->get_modules())
                {
                    const u32 module_id = module->get_id();
                    c_grouping->assign_module_by_id(module_id);
                }

                for (const Net* net : grouping->get_nets())
                {
                    const u32 net_id = net->get_id();
                    c_grouping->assign_net_by_id(net_id);
                }

                for (const Gate* gate : grouping->get_gates())
                {
                    const u32 gate_id = gate->get_id();
                    c_grouping->assign_gate_by_id(gate_id);
                }
            }

            // mark globals
            for (const Net* global_input_net : nl->get_global_input_nets())
            {
                Net* c_global_input_net = c_netlist->get_net_by_id(global_input_net->get_id());
                c_netlist->mark_global_input_net(c_global_input_net);
            }
            for (const Net* global_output_net : nl->get_global_output_nets())
            {
                Net* c_global_output_net = c_netlist->get_net_by_id(global_output_net->get_id());
                c_netlist->mark_global_output_net(c_global_output_net);
            }
            for (const Gate* gnd_gate : nl->get_gnd_gates())
            {
                Gate* c_gnd_gate = c_netlist->get_gate_by_id(gnd_gate->get_id());
                c_netlist->mark_gnd_gate(c_gnd_gate);
            }
            for (const Gate* vcc_gate : nl->get_vcc_gates())
            {
                Gate* c_vcc_gate = c_netlist->get_gate_by_id(vcc_gate->get_id());
                c_netlist->mark_vcc_gate(c_vcc_gate);
            }

            c_netlist->set_design_name(nl->get_design_name());
            c_netlist->set_device_name(nl->get_device_name());
            c_netlist->set_input_filename(nl->get_input_filename());

            // update ids last, after all the creation
            c_netlist->set_next_gate_id(nl->get_next_gate_id());
            c_netlist->set_used_gate_ids(nl->get_used_gate_ids());
            c_netlist->set_free_gate_ids(nl->get_free_gate_ids());

            c_netlist->set_next_net_id(nl->get_next_net_id());
            c_netlist->set_used_net_ids(nl->get_used_net_ids());
            c_netlist->set_free_net_ids(nl->get_free_net_ids());

            c_netlist->set_next_module_id(nl->get_next_module_id());
            c_netlist->set_used_module_ids(nl->get_used_module_ids());
            c_netlist->set_free_module_ids(nl->get_free_module_ids());

            c_netlist->set_next_grouping_id(nl->get_next_grouping_id());
            c_netlist->set_used_grouping_ids(nl->get_used_grouping_ids());
            c_netlist->set_free_grouping_ids(nl->get_free_grouping_ids());

            // copy module port names
            for (const Module* module : nl->get_modules())
            {
                const u32 module_id = module->get_id();
                Module* c_module    = c_netlist->get_module_by_id(module_id);

                for (const auto& [net, port_name] : module->get_input_port_names())
                {
                    u32 net_id = net->get_id();
                    c_module->set_input_port_name(c_netlist->get_net_by_id(net_id), port_name);
                }

                for (const auto& [net, port_name] : module->get_output_port_names())
                {
                    u32 net_id = net->get_id();
                    c_module->set_output_port_name(c_netlist->get_net_by_id(net_id), port_name);
                }

                c_module->set_next_input_port_id(module->get_next_input_port_id());
                c_module->set_next_output_port_id(module->get_next_output_port_id());
            }

            return c_netlist;
        }
    }    // namespace netlist_utils
}    // namespace hal
