#include "hal_core/netlist/netlist_utils.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"

#include <deque>
#include <queue>
#include <unordered_set>

namespace hal
{
    namespace netlist_utils
    {
        namespace
        {
            std::vector<Gate*> get_shortest_path_internal(Gate* start_gate, Gate* end_gate)
            {
                if (start_gate == end_gate) return std::vector<Gate*>();
                std::vector<Gate*> v0;
                v0.push_back(start_gate);
                std::unordered_map<Gate*,Gate*> originMap;
                for (;;)
                {
                    std::vector<Gate*> v1;
                    for (Gate* g0 : v0)
                    {
                        for (Gate* g1 : get_next_gates(g0, true, 1))
                        {
                            if (originMap.find(g1) != originMap.end()) continue; // already routed to
                            v1.push_back(g1);
                            originMap[g1] = g0;
                            if (g1 == end_gate)
                            {
                                // heureka!
                                std::vector<Gate*> retval;
                                Gate* g = end_gate;
                                while (g != start_gate)
                                {
                                    retval.insert(retval.begin(),g);
                                    auto it = originMap.find(g);
                                    assert(it != originMap.end());
                                    g = it->second;
                                }
                                retval.insert(retval.begin(),start_gate);
                                return retval;
                            }
                        }
                    }
                    if (v1.empty()) break;
                    v0 = v1;
                }
                return std::vector<Gate*>();
            }

            static BooleanFunction get_function_of_gate(const Gate* const gate, std::unordered_map<u32, BooleanFunction>& cache)
            {
                if (auto it = cache.find(gate->get_id()); it != cache.end())
                {
                    return it->second;
                }
                else
                {
                    BooleanFunction bf = gate->get_boolean_function();

                    for (const std::string& input_pin : bf.get_variables())
                    {
                        const Net* const input_net = gate->get_fan_in_net(input_pin);
                        if (input_net == nullptr)
                        {
                            // if no net is connected, the input pin name cannot be replaced
                            log_warning("netlist_utils", "not net is connected to input pin '{}' of gate with ID {}, cannot replace pin name with net ID.", input_pin, gate->get_id());
                            return bf;
                        }

                        bf = bf.substitute(input_pin, std::to_string(input_net->get_id()));
                    }
                    cache.emplace(gate->get_id(), bf);
                    return bf;
                }
            }

            void subgraph_function_bfs(Net* n, BooleanFunction& result, std::vector<Net*> stack, const std::vector<const Gate*>& subgraph_gates, std::unordered_map<u32, BooleanFunction>& cache)
            {
                if (n->get_num_of_sources() > 1)
                {
                    log_error("netlist_utils", "net with ID {} has more than one source, cannot expand Boolean function in this direction.", n->get_id());
                    return;
                }
                else if (n->get_num_of_sources() == 0)
                {
                    return;
                }

                if (auto it = std::find(stack.begin(), stack.end(), n); it != stack.end())
                {
                    log_error("netlist_utils", "subgraph contains a cycle: {} -> {}", utils::join(" -> ", it, stack.end(), [](auto nlog) { return nlog->get_name(); }), n->get_name());
                    result = BooleanFunction();
                    return;
                }

                stack.push_back(n);

                Gate* src_gate = n->get_sources()[0]->get_gate();

                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) != subgraph_gates.end())
                {
                    result = result.substitute(std::to_string(n->get_id()), get_function_of_gate(src_gate, cache));

                    for (Net* sn : src_gate->get_fan_in_nets())
                    {
                        subgraph_function_bfs(sn, result, stack, subgraph_gates, cache);
                    }
                }
            }
        }    // namespace

        BooleanFunction get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates, std::unordered_map<u32, BooleanFunction>& cache)
        {
            /* check validity of subgraph_gates */
            if (subgraph_gates.empty())
            {
                log_error("netlist_utils", "no gates given to determine the Boolean function of.");
                return BooleanFunction();
            }
            else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
            {
                log_error("netlist_utils", "set of gates contains a nullptr.");
                return BooleanFunction();
            }
            else if (net == nullptr)
            {
                log_error("netlist_utils", "nullptr given for target net.");
                return BooleanFunction();
            }
            else if (net->get_num_of_sources() > 1)
            {
                log_error("netlist_utils", "target net with ID {} has more than one source.", net->get_id());
                return BooleanFunction();
            }
            else if (net->get_num_of_sources() == 0)
            {
                log_error("netlist_utils", "target net with ID {} has no sources.", net->get_id());
                return BooleanFunction();
            }

            const Gate* start_gate = net->get_sources()[0]->get_gate();
            BooleanFunction result = get_function_of_gate(start_gate, cache);

            for (Net* n : start_gate->get_fan_in_nets())
            {
                subgraph_function_bfs(n, result, {}, subgraph_gates, cache);
            }

            return result;
        }

        BooleanFunction get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates)
        {
            std::unordered_map<u32, BooleanFunction> cache;
            return get_subgraph_function(net, subgraph_gates, cache);
        }

        std::unique_ptr<Netlist> copy_netlist(const Netlist* nl)
        {
            std::unique_ptr<Netlist> c_netlist = netlist_factory::create_netlist(nl->get_gate_library());

            // manager, netlist_id, and top_module are set in the constructor

            // copy nets
            for (const Net* net : nl->get_nets())
            {
                Net* new_net = c_netlist->create_net(net->get_id(), net->get_name());
                new_net->set_data_map(net->get_data_map());
            }

            // copy gates
            for (const Gate* gate : nl->get_gates())
            {
                Gate* new_gate = c_netlist->create_gate(gate->get_id(), gate->get_type(), gate->get_name(), gate->get_location_x(), gate->get_location_y());

                for (const auto& [name, func] : gate->get_boolean_functions(true))
                {
                    new_gate->add_boolean_function(name, func);
                }

                for (const Endpoint* in_point : gate->get_fan_in_endpoints())
                {
                    const auto net_id = in_point->get_net()->get_id();
                    auto c_net        = c_netlist->get_net_by_id(net_id);

                    c_net->add_destination(new_gate, in_point->get_pin());
                }

                for (const Endpoint* out_point : gate->get_fan_out_endpoints())
                {
                    const auto net_id = out_point->get_net()->get_id();
                    auto c_net        = c_netlist->get_net_by_id(net_id);

                    c_net->add_source(new_gate, out_point->get_pin());
                }

                new_gate->set_data_map(gate->get_data_map());
            }

            // copy modules
            for (const Module* module : nl->get_modules())
            {
                // ignore top module, since this is already created by the constructor
                if (module->get_id() == 1)
                {
                    c_netlist->get_top_module()->set_data_map(module->get_data_map());
                    c_netlist->get_top_module()->set_type(module->get_type());
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
                Module* new_module = c_netlist->create_module(module->get_id(), module->get_name(), c_netlist->get_top_module(), c_gates);

                new_module->set_data_map(module->get_data_map());
                new_module->set_type(module->get_type());
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

        std::vector<Gate*> get_next_gates(const Gate* gate, bool get_successors, int depth, const std::function<bool(const Gate*)>& filter)
        {
            std::vector<Gate*> retval;
            std::unordered_map<u32, std::vector<Gate*>> cache;
            std::vector<const Gate*> v0;
            v0.push_back(gate);
            std::unordered_set<const Gate*> gats_handled;
            std::unordered_set<const Net*> nets_handled;
            gats_handled.insert(gate);

            for (int round = 0; !depth || round < depth; round++)
            {
                std::vector<const Gate*> v1;
                for (const Gate* g0 : v0)
                {
                    for (const Net* n : get_successors ? g0->get_fan_out_nets() : g0->get_fan_in_nets())
                    {
                        if (nets_handled.find(n) != nets_handled.end()) continue;
                        nets_handled.insert(n);

                        for (const Endpoint* ep : get_successors ? n->get_destinations() : n->get_sources())
                        {
                            Gate* g1 = ep->get_gate();
                            if (gats_handled.find(g1) != gats_handled.end()) continue; // already handled
                            gats_handled.insert(g1);
                            v1.push_back(g1);
                            if (!filter || filter(g1)) retval.push_back(g1);
                        }
                    }
                }
                if (v1.empty()) break;
                v0 = v1;
            }
            return retval;
        }

        std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions)
        {
            std::vector<Gate*> path_forward = get_shortest_path_internal(start_gate, end_gate);
            if (!search_both_directions) return path_forward;
            std::vector<Gate*> path_reverse = get_shortest_path_internal(end_gate, start_gate);
            return (path_reverse.size() < path_forward.size()) ? path_reverse : path_forward;
        }

        namespace
        {
            std::vector<Gate*> get_next_sequential_gates_internal(const Net* start_net, bool forward, std::unordered_set<u32>& seen, std::unordered_map<u32, std::vector<Gate*>>& cache)
            {
                if (auto it = cache.find(start_net->get_id()); it != cache.end())
                {
                    return it->second;
                }

                if (seen.find(start_net->get_id()) != seen.end())
                {
                    return {};
                }

                seen.insert(start_net->get_id());

                std::vector<Gate*> found_ffs;

                for (auto endpoint : forward ? start_net->get_destinations() : start_net->get_sources())
                {
                    auto next_gate = endpoint->get_gate();

                    if (next_gate->get_type()->has_property(GateTypeProperty::ff))
                    {
                        found_ffs.push_back(next_gate);
                    }
                    else
                    {
                        for (auto n : forward ? next_gate->get_fan_out_nets() : next_gate->get_fan_in_nets())
                        {
                            auto next_gates = get_next_sequential_gates_internal(n, forward, seen, cache);
                            found_ffs.insert(found_ffs.end(), next_gates.begin(), next_gates.end());
                        }
                    }
                }

                std::sort(found_ffs.begin(), found_ffs.end());
                found_ffs.erase(std::unique(found_ffs.begin(), found_ffs.end()), found_ffs.end());

                cache.emplace(start_net->get_id(), found_ffs);
                return found_ffs;
            }
        }    // namespace

        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::vector<Gate*> found_ffs;
            for (const auto& n : get_successors ? gate->get_fan_out_nets() : gate->get_fan_in_nets())
            {
                auto suc = get_next_sequential_gates(n, get_successors, cache);
                found_ffs.insert(found_ffs.end(), suc.begin(), suc.end());
            }

            std::sort(found_ffs.begin(), found_ffs.end());
            found_ffs.erase(std::unique(found_ffs.begin(), found_ffs.end()), found_ffs.end());

            return found_ffs;
        }

        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::unordered_set<u32> seen;
            return get_next_sequential_gates_internal(net, get_successors, seen, cache);
        }

        std::vector<Gate*> get_next_sequential_gates(const Gate* gate, bool get_successors)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_next_sequential_gates(gate, get_successors, cache);
        }

        std::vector<Gate*> get_next_sequential_gates(const Net* net, bool get_successors)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_next_sequential_gates(net, get_successors, cache);
        }

        namespace
        {
            std::vector<Gate*>
                get_path_internal(const Net* start_net, bool forward, std::set<GateTypeProperty> stop_types, std::unordered_set<u32>& seen, std::unordered_map<u32, std::vector<Gate*>>& cache)
            {
                if (auto it = cache.find(start_net->get_id()); it != cache.end())
                {
                    return it->second;
                }

                if (seen.find(start_net->get_id()) != seen.end())
                {
                    return {};
                }

                seen.insert(start_net->get_id());

                std::vector<Gate*> found_combinational;

                for (auto endpoint : forward ? start_net->get_destinations() : start_net->get_sources())
                {
                    auto next_gate = endpoint->get_gate();

                    bool stop = false;
                    for (GateTypeProperty property : next_gate->get_type()->get_properties())
                    {
                        if (stop_types.find(property) != stop_types.end())
                        {
                            stop = true;
                        }
                    }

                    if (stop == false)
                    {
                        found_combinational.push_back(next_gate);

                        for (auto n : forward ? next_gate->get_fan_out_nets() : next_gate->get_fan_in_nets())
                        {
                            auto next_gates = get_path_internal(n, forward, stop_types, seen, cache);
                            found_combinational.insert(found_combinational.end(), next_gates.begin(), next_gates.end());
                        }
                    }
                }

                std::sort(found_combinational.begin(), found_combinational.end());
                found_combinational.erase(std::unique(found_combinational.begin(), found_combinational.end()), found_combinational.end());

                cache.emplace(start_net->get_id(), found_combinational);
                return found_combinational;
            }
        }    // namespace

        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::vector<Gate*> found_combinational;
            for (const auto& n : get_successors ? gate->get_fan_out_nets() : gate->get_fan_in_nets())
            {
                auto suc = get_path(n, get_successors, stop_properties, cache);
                found_combinational.insert(found_combinational.end(), suc.begin(), suc.end());
            }

            std::sort(found_combinational.begin(), found_combinational.end());
            found_combinational.erase(std::unique(found_combinational.begin(), found_combinational.end()), found_combinational.end());

            return found_combinational;
        }

        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache)
        {
            std::unordered_set<u32> seen;
            return get_path_internal(net, get_successors, stop_properties, seen, cache);
        }

        std::vector<Gate*> get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_path(gate, get_successors, stop_properties, cache);
        }

        std::vector<Gate*> get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties)
        {
            std::unordered_map<u32, std::vector<Gate*>> cache;
            return get_path(net, get_successors, stop_properties, cache);
        }

        std::unordered_set<Net*> get_nets_at_pins(Gate* gate, std::unordered_set<std::string> pins, bool is_inputs)
        {
            std::unordered_set<Net*> nets;

            if (is_inputs)
            {
                for (const auto& pin : pins)
                {
                    if (Net* net = gate->get_fan_in_net(pin); net != nullptr)
                    {
                        nets.insert(net);
                    }
                    else
                    {
                        log_error("netlist_utils", "could not retrieve fan-in net for pin '{}' of gate '{}' with ID {}.", pin, gate->get_name(), gate->get_id());
                    }
                }
            }
            else
            {
                for (const auto& pin : pins)
                {
                    if (Net* net = gate->get_fan_out_net(pin); net != nullptr)
                    {
                        nets.insert(net);
                    }
                    else
                    {
                        log_error("netlist_utils", "could not retrieve fan-out net for pin '{}' of gate '{}' with ID {}.", pin, gate->get_name(), gate->get_id());
                    }
                }
            }
            return nets;
        }

        void remove_buffers(Netlist* netlist, bool analyze_inputs)
        {
            u32 num_gates = 0;

            Net* gnd_net = netlist->get_gnd_gates().front()->get_fan_out_nets().front();
            Net* vcc_net = netlist->get_vcc_gates().front()->get_fan_out_nets().front();

            for (const auto& gate : netlist->get_gates())
            {
                std::vector<Endpoint*> fan_out = gate->get_fan_out_endpoints();

                GateType* gt = gate->get_type();
                if (!gt->has_property(GateTypeProperty::combinational) || gt->has_property(GateTypeProperty::power) || gt->has_property(GateTypeProperty::ground))
                {
                    // continue if of invalid base type
                    continue;
                }

                if (fan_out.size() != 1)
                {
                    // continue if more than one fan-out net
                    continue;
                }

                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();
                if (functions.size() != 1)
                {
                    // continue if more than one Boolean function (tri-state?)
                    continue;
                }

                Endpoint* out_endpoint = *(fan_out.begin());
                if (out_endpoint->get_pin() != (functions.begin())->first)
                {
                    // continue if Boolean function name does not match output pin
                    continue;
                }

                std::vector<Endpoint*> fan_in = gate->get_fan_in_endpoints();
                BooleanFunction func          = functions.begin()->second;

                if (analyze_inputs)
                {
                    for (Endpoint* ep : fan_in)
                    {
                        auto sources = ep->get_net()->get_sources();
                        if (sources.size() != 1)
                        {
                            break;
                        }

                        if (sources.front()->get_gate()->is_gnd_gate())
                        {
                            func = func.substitute(ep->get_pin(), BooleanFunction::Value::ZERO);
                        }
                        else if (sources.front()->get_gate()->is_vcc_gate())
                        {
                            func = func.substitute(ep->get_pin(), BooleanFunction::Value::ONE);
                        }
                    }

                    func = func.optimize_constants();
                }

                std::string func_str                    = func.to_string();
                std::unordered_set<std::string> in_pins = gt->get_pins_of_direction(PinDirection::input);
                if (in_pins.find(func_str) != in_pins.end())
                {
                    Net* out_net = out_endpoint->get_net();

                    // check all input endpoints and ...
                    for (Endpoint* in_endpoint : fan_in)
                    {
                        Net* in_net = in_endpoint->get_net();

                        if (in_endpoint->get_pin() == func_str)
                        {
                            // reconnect outputs if the input is passed through the buffer
                            for (Endpoint* dst : out_net->get_destinations())
                            {
                                Gate* dst_gate      = dst->get_gate();
                                std::string dst_pin = dst->get_pin();
                                out_net->remove_destination(dst);
                                in_net->add_destination(dst_gate, dst_pin);
                            }
                        }
                        else
                        {
                            // remove the input endpoint otherwise
                            in_net->remove_destination(gate, in_endpoint->get_pin());
                        }
                    }

                    // delete output net and buffer gate
                    netlist->delete_net(out_net);
                    netlist->delete_gate(gate);
                    num_gates++;
                }
                else if (func_str == "0" || func_str == "1")
                {
                    Net* out_net = out_endpoint->get_net();

                    for (Endpoint* in_endpoint : fan_in)
                    {
                        // remove the input endpoint otherwise
                        in_endpoint->get_net()->remove_destination(gate, in_endpoint->get_pin());
                    }
                    if (func_str == "0")
                    {
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate      = dst->get_gate();
                            std::string dst_pin = dst->get_pin();
                            out_net->remove_destination(dst);
                            gnd_net->add_destination(dst_gate, dst_pin);
                        }
                    }
                    else if (func_str == "1")
                    {
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate      = dst->get_gate();
                            std::string dst_pin = dst->get_pin();
                            out_net->remove_destination(dst);
                            vcc_net->add_destination(dst_gate, dst_pin);
                        }
                    }

                    // delete output net and buffer gate
                    netlist->delete_net(out_net);
                    netlist->delete_gate(gate);
                    num_gates++;
                }
            }

            log_info("netlist_utils", "removed {} buffer gates from the netlist.", num_gates);
        }

        void remove_unused_lut_endpoints(Netlist* netlist)
        {
            u32 num_eps = 0;

            // net connected to GND
            Net* gnd_net = *(*netlist->get_gnd_gates().begin())->get_fan_out_nets().begin();

            // iterate all LUT gates
            for (const auto& gate : netlist->get_gates([](Gate* g) { return g->get_type()->has_property(GateTypeProperty::lut); }))
            {
                std::vector<Endpoint*> fan_in                              = gate->get_fan_in_endpoints();
                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();

                // skip if more than one function
                if (functions.size() != 1)
                {
                    continue;
                }

                std::vector<std::string> active_pins = functions.begin()->second.get_variables();

                // if there are more fan-in nets than there are active pins, we need to get rid of some nets
                if (fan_in.size() > active_pins.size())
                {
                    for (const auto& ep : fan_in)
                    {
                        if (std::find(active_pins.begin(), active_pins.end(), ep->get_pin()) == active_pins.end())
                        {
                            num_eps++;
                            std::string pin = ep->get_pin();
                            ep->get_net()->remove_destination(gate, pin);
                            gnd_net->add_destination(gate, pin);
                        }
                    }
                }
            }

            log_info("netlist_utils", "removed {} unused LUT fan-in endpoints from the netlist.", num_eps);
        }

        void rename_luts_according_to_function(Netlist* netlist)
        {
            u32 num_luts = 0;

            std::unordered_map<std::string, std::string> truth_table_to_function = {
                {"01", "BUF"},
                {"10", "INV"},
                {"0001", "AND2"},
                {"1110", "NAND2"},
                {"0111", "OR2"},
                {"1000", "NOR2"},
                {"0110", "XOR2"},
                {"1001", "XNOR2"},
                {"00000001", "AND3"},
                {"11111110", "NAND3"},
                {"01111111", "OR3"},
                {"10000000", "NOR3"},
                {"01101001", "XOR3"},
                {"10010110", "XNOR3"},
                {"00011011", "MUX"},      // A B S_B (meaning S selects B when S=1)
                {"00100111", "MUX"},      // A B S_A
                {"00011101", "MUX"},      // A S_B B
                {"01000111", "MUX"},      // A S_A B
                {"00110101", "MUX"},      // S_B A B
                {"01010011", "MUX"},      // S_A A B
                {"11100000", "AOI21"},    // !(A | (B & C))
                {"11001000", "AOI21"},    // !(B | (A & C))
                {"10101000", "AOI21"},    // !(C | (A & B))
                {"11111000", "OAI21"},    // !(A | (B & C))
                {"11101100", "OAI21"},    // !(B | (A & C))
                {"11101010", "OAI21"},    // !(C | (A & B))
                {"0000000000000001", "AND4"},
                {"1111111111111110", "NAND4"},
                {"0111111111111111", "OR4"},
                {"1000000000000000", "NOR4"},
                {"0110100110010110", "XOR4"},
                {"1001011001101001", "XNOR4"},
                {"1110111011100000", "AOI22"},    // !((A & B) | (C & D))
                {"1111101011001000", "AOI22"},    // !((A & C) | (B & D))
                {"1111110010101000", "AOI22"},    // !((A & D) | (B & C))
                {"1111100010001000", "OAI22"},    // !((A | B) & (C | D))
                {"1110110010100000", "OAI22"},    // !((A | C) & (B | D))
                {"1110101011000000", "OAI22"}     // !((A | D) & (B | C))
            };

            for (Gate* gate : netlist->get_gates([](Gate* g) { return g->get_type()->has_property(GateTypeProperty::lut); }))
            {
                std::unordered_map<std::string, BooleanFunction> functions = gate->get_boolean_functions();

                if (functions.size() != 1)
                {
                    continue;
                }

                std::string truth_table = "";
                for (BooleanFunction::Value val : functions.begin()->second.get_truth_table())
                {
                    truth_table += BooleanFunction::to_string(val);
                }

                if (auto it = truth_table_to_function.find(truth_table); it != truth_table_to_function.end())
                {
                    gate->set_name(it->second + "_" + std::to_string(gate->get_id()));
                    num_luts++;
                }
            }

            log_info("netlist_utils", "renamed {} LUTs according to the function they implement.", num_luts);
        }

        std::vector<Net*> get_common_inputs(const std::vector<Gate*>& gates, u32 threshold)
        {
            // if threshold = 0, a net is only considered to be common if it is an input to all gates
            if (threshold == 0)
            {
                threshold = gates.size();
            }

            // count input net occurences
            std::map<Net*, u32> net_count;
            for (Gate* g : gates)
            {
                for (Endpoint* pred : g->get_predecessors())
                {
                    if (pred->get_gate()->is_gnd_gate() || pred->get_gate()->is_vcc_gate())
                    {
                        continue;
                    }

                    Net* pred_net = pred->get_net();
                    if (const auto it = net_count.find(pred_net); it != net_count.end())
                    {
                        it->second++;
                    }
                    else
                    {
                        net_count[pred_net] = 1;
                    }
                }
            }

            // consider every net that is input to at least half the gates to be a common input
            std::vector<Net*> common_inputs;
            for (const auto& [n, cnt] : net_count)
            {
                if (cnt >= threshold)
                {
                    common_inputs.push_back(n);
                }
            }

            return common_inputs;
        }

        bool replace_gate(Gate* gate, GateType* target_type, std::map<std::string, std::string> pin_map)
        {
            Netlist* netlist                  = gate->get_netlist();
            u32 gate_id                       = gate->get_id();
            std::string gate_name             = gate->get_name();
            std::pair<i32, i32> gate_location = gate->get_location();
            std::vector<Endpoint*> fan_in     = gate->get_fan_in_endpoints();
            std::vector<Endpoint*> fan_out    = gate->get_fan_out_endpoints();
            Module* gate_module               = gate->get_module();
            Grouping* gate_grouping           = gate->get_grouping();
            auto gate_data                    = gate->get_data_map();

            std::map<std::string, Net*> in_nets;
            std::map<std::string, Net*> out_nets;

            // map new input pins to nets
            for (Endpoint* in_ep : fan_in)
            {
                if (const auto it = pin_map.find(in_ep->get_pin()); it != pin_map.end())
                {
                    in_nets[it->second] = in_ep->get_net();
                }
            }

            for (Endpoint* out_ep : fan_out)
            {
                if (const auto it = pin_map.find(out_ep->get_pin()); it != pin_map.end())
                {
                    out_nets[it->second] = out_ep->get_net();
                }
            }

            // remove old gate
            netlist->delete_gate(gate);

            // create new gate
            Gate* new_gate = netlist->create_gate(gate_id, target_type, gate_name, gate_location.first, gate_location.second);
            if (new_gate == nullptr)
            {
                log_error("netlist_utils",
                          "failed to replace gate '{}' with ID {} in netlist with ID {} with new gate of type '{}' after deleting the original gate.",
                          gate_name,
                          gate_id,
                          netlist->get_id(),
                          target_type->get_name());
                return false;
            }

            // reconnect nets
            for (const auto& [in_pin, in_net] : in_nets)
            {
                if (in_net->add_destination(new_gate, in_pin) == nullptr)
                {
                    log_error("netlist_utils",
                              "failed to reconnect input net '{}' with ID {} to pin '{}' of the replacement gate '{}' with ID {} of type '{}' in netlist with ID {}.",
                              in_net->get_name(),
                              in_net->get_id(),
                              in_pin,
                              gate_name,
                              gate_id,
                              target_type->get_name(),
                              netlist->get_id());
                    return false;
                }
            }

            for (const auto& [out_pin, out_net] : out_nets)
            {
                if (out_net->add_source(new_gate, out_pin) == nullptr)
                {
                    log_error("netlist_utils",
                              "failed to reconnect output net '{}' with ID {} to pin '{}' of the replacement gate '{}' with ID {} of type '{}' in netlist with ID {}.",
                              out_net->get_name(),
                              out_net->get_id(),
                              out_pin,
                              gate_name,
                              gate_id,
                              target_type->get_name(),
                              netlist->get_id());
                    return false;
                }
            }

            // restore data, module, and grouping
            if (!gate_module->is_top_module())
            {
                gate_module->assign_gate(new_gate);
            }
            if (gate_grouping != nullptr)
            {
                gate_grouping->assign_gate(new_gate);
            }
            if (!gate_data.empty())
            {
                new_gate->set_data_map(gate_data);
            }

            return true;
        }

        std::vector<Gate*> get_gate_chain(Gate* start_gate, const std::set<std::string>& input_pins, const std::set<std::string>& output_pins, const std::function<bool(const Gate*)>& filter)
        {
            // check filter on start gate
            if (filter && !filter(start_gate))
            {
                return {};
            }

            std::deque<Gate*> gate_chain            = {start_gate};
            std::unordered_set<Gate*> visited_gates = {start_gate};
            const GateType* target_type             = start_gate->get_type();
            bool found_next_gate;

            // move forward
            const Gate* current_gate = start_gate;
            do
            {
                found_next_gate = false;

                // check all eligable successors of current gate
                std::vector<Endpoint*> successors = current_gate->get_successors([input_pins, output_pins, target_type, filter](const std::string& ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (output_pins.empty() || output_pins.find(ep_pin) != output_pins.end())
                        {
                            if (input_pins.empty() || input_pins.find(ep->get_pin()) != input_pins.end())
                            {
                                if (!filter || filter(ep->get_gate()))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                });

                if (successors.size() > 1)
                {
                    log_debug("netlist_utils",
                              "detected more than one valid successor gate for gate '{}' with ID {} in netlist with ID {}.",
                              current_gate->get_name(),
                              current_gate->get_id(),
                              current_gate->get_netlist()->get_id());
                    break;
                }
                else if (!successors.empty())
                {
                    Gate* suc_gate = successors.at(0)->get_gate();

                    if (visited_gates.find(suc_gate) != visited_gates.end())
                    {
                        log_debug("netlist_utils", "detected a loop at gate with ID {}.", suc_gate->get_id());
                        break;
                    }

                    gate_chain.push_back(suc_gate);
                    visited_gates.insert(suc_gate);
                    current_gate    = suc_gate;
                    found_next_gate = true;
                }
            } while (found_next_gate);

            // move backwards
            current_gate = start_gate;
            do
            {
                found_next_gate = false;

                // check all eligable predecessors of current gate
                std::vector<Endpoint*> predecessors = current_gate->get_predecessors([input_pins, output_pins, target_type, filter](const std::string& ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (input_pins.empty() || input_pins.find(ep_pin) != input_pins.end())
                        {
                            if (output_pins.empty() || output_pins.find(ep->get_pin()) != output_pins.end())
                            {
                                if (!filter || filter(ep->get_gate()))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                });

                if (predecessors.size() > 1)
                {
                    log_debug("netlist_utils",
                              "detected more than one valid predecessor gate for gate '{}' with ID {} in netlist with ID {}.",
                              current_gate->get_name(),
                              current_gate->get_id(),
                              current_gate->get_netlist()->get_id());
                    break;
                }
                else if (!predecessors.empty())
                {
                    Gate* pred_gate = predecessors.at(0)->get_gate();

                    if (visited_gates.find(pred_gate) != visited_gates.end())
                    {
                        log_debug("netlist_utils", "detected a loop at gate with ID {}.", pred_gate->get_id());
                        break;
                    }

                    gate_chain.push_front(pred_gate);
                    visited_gates.insert(pred_gate);
                    current_gate    = pred_gate;
                    found_next_gate = true;
                    log_debug("netlist_utils", "found predecessor gate with ID {}.", pred_gate->get_id());
                }
            } while (found_next_gate);

            return std::vector<Gate*>(gate_chain.begin(), gate_chain.end());
        }

        std::vector<Gate*> get_complex_gate_chain(Gate* start_gate,
                                                  const std::vector<GateType*>& chain_types,
                                                  const std::map<GateType*, std::set<std::string>>& input_pins,
                                                  const std::map<GateType*, std::set<std::string>>& output_pins,
                                                  const std::function<bool(const Gate*)>& filter)
        {
            if (start_gate->get_type() != chain_types.at(0) || (filter && !filter(start_gate)))
            {
                return {};
            }

            std::deque<Gate*> gate_chain = {start_gate};
            std::unordered_set<Gate*> visited_gates;

            u32 last_index    = 0;
            u32 current_index = (last_index + 1) % chain_types.size();

            // move forward
            bool found_next_gate;
            const Gate* current_gate = start_gate;
            do
            {
                found_next_gate = false;

                // check all successors of current gate
                GateType* target_type                = chain_types.at(current_index);
                const std::set<std::string>& inputs  = input_pins.at(target_type);
                const std::set<std::string>& outputs = output_pins.at(chain_types.at(last_index));
                std::vector<Endpoint*> successors    = current_gate->get_successors([target_type, inputs, outputs, filter](const std::string& ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (outputs.empty() || outputs.find(ep_pin) != outputs.end())
                        {
                            if (inputs.empty() || inputs.find(ep->get_pin()) != inputs.end())
                            {
                                if (!filter || filter(ep->get_gate()))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                });

                if (successors.size() > 1)
                {
                    log_debug("netlist_utils",
                              "detected more than one valid successor gate for gate '{}' with ID {} in netlist with ID {}.",
                              current_gate->get_name(),
                              current_gate->get_id(),
                              current_gate->get_netlist()->get_id());
                    break;
                }
                else if (!successors.empty())
                {
                    Gate* suc_gate = successors.at(0)->get_gate();

                    if (visited_gates.find(suc_gate) != visited_gates.end())
                    {
                        log_debug("netlist_utils", "detected a loop at gate with ID {}.", suc_gate->get_id());
                        break;
                    }

                    gate_chain.push_back(suc_gate);
                    visited_gates.insert(suc_gate);
                    current_gate    = suc_gate;
                    last_index      = current_index;
                    current_index   = (current_index + 1) % chain_types.size();
                    found_next_gate = true;
                }
            } while (found_next_gate);

            // remove partial sequences at the end of the chain
            while (current_index != 0)
            {
                gate_chain.pop_back();
                current_index--;
            }

            current_gate  = start_gate;
            last_index    = 0;
            current_index = chain_types.size() - 1;

            // move backwards
            do
            {
                found_next_gate = false;

                // check all predecessors of current gate
                GateType* target_type                = chain_types.at(current_index);
                const std::set<std::string>& inputs  = input_pins.at(chain_types.at(last_index));
                const std::set<std::string>& outputs = output_pins.at(target_type);
                std::vector<Endpoint*> predecessors  = current_gate->get_predecessors([target_type, inputs, outputs, filter](const std::string& ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (inputs.empty() || inputs.find(ep_pin) != inputs.end())
                        {
                            if (outputs.empty() || outputs.find(ep->get_pin()) != outputs.end())
                            {
                                if (!filter || filter(ep->get_gate()))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                });

                if (predecessors.size() > 1)
                {
                    log_debug("netlist_utils",
                              "detected more than one valid predecessor gate for gate '{}' with ID {} in netlist with ID {}.",
                              current_gate->get_name(),
                              current_gate->get_id(),
                              current_gate->get_netlist()->get_id());
                    break;
                }
                else if (!predecessors.empty())
                {
                    Gate* pred_gate = predecessors.at(0)->get_gate();

                    if (visited_gates.find(pred_gate) != visited_gates.end())
                    {
                        log_debug("netlist_utils", "detected a loop at gate with ID {}.", pred_gate->get_id());
                        break;
                    }

                    gate_chain.push_front(pred_gate);
                    visited_gates.insert(pred_gate);
                    current_gate    = pred_gate;
                    last_index      = current_index;
                    current_index   = (current_index == 0) ? chain_types.size() - 1 : current_index - 1;
                    found_next_gate = true;
                }
            } while (found_next_gate);

            // remove partial sequences at the beginning of the chain
            while (last_index != 0)
            {
                gate_chain.pop_front();
                last_index--;
            }

            return std::vector<Gate*>(gate_chain.begin(), gate_chain.end());
        }
    }    // namespace netlist_utils
}    // namespace hal
