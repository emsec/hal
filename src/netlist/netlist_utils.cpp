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
                if (start_gate == end_gate)
                    return std::vector<Gate*>();
                std::vector<Gate*> v0;
                v0.push_back(start_gate);
                std::unordered_map<Gate*, Gate*> originMap;
                for (;;)
                {
                    std::vector<Gate*> v1;
                    for (Gate* g0 : v0)
                    {
                        for (Gate* g1 : get_next_gates(g0, true, 1))
                        {
                            if (originMap.find(g1) != originMap.end())
                                continue;    // already routed to
                            v1.push_back(g1);
                            originMap[g1] = g0;
                            if (g1 == end_gate)
                            {
                                // heureka!
                                std::vector<Gate*> retval;
                                Gate* g = end_gate;
                                while (g != start_gate)
                                {
                                    retval.insert(retval.begin(), g);
                                    auto it = originMap.find(g);
                                    assert(it != originMap.end());
                                    g = it->second;
                                }
                                retval.insert(retval.begin(), start_gate);
                                return retval;
                            }
                        }
                    }
                    if (v1.empty())
                        break;
                    v0 = v1;
                }
                return std::vector<Gate*>();
            }

            static Result<BooleanFunction> get_function_of_gate(const Gate* const gate, const GatePin* output_pin, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
            {
                if (auto it = cache.find({gate->get_id(), output_pin}); it != cache.end())
                {
                    return OK(it->second);
                }
                else
                {
                    BooleanFunction bf = gate->get_boolean_function(output_pin);

                    std::vector<std::string> input_vars = utils::to_vector(bf.get_variable_names());
                    while (!input_vars.empty())
                    {
                        const std::string var = input_vars.back();
                        input_vars.pop_back();

                        const GatePin* pin = gate->get_type()->get_pin_by_name(var);
                        if (pin == nullptr)
                        {
                            return ERR("could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to get input pin '" + var
                                       + "' by name");
                        }

                        const PinDirection pin_dir = pin->get_direction();
                        if (pin_dir == PinDirection::input)
                        {
                            const Net* const input_net = gate->get_fan_in_net(var);
                            if (input_net == nullptr)
                            {
                                // if no net is connected, the input pin name cannot be replaced
                                return ERR("could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to get fan-in net at pin '"
                                           + pin->get_name() + "'");
                            }

                            bf = bf.substitute(var, "net_" + std::to_string(input_net->get_id()));
                        }
                        else if ((pin_dir == PinDirection::internal) || (pin_dir == PinDirection::output))
                        {
                            BooleanFunction bf_interal = gate->get_boolean_function(var);
                            if (bf_interal.is_empty())
                            {
                                return ERR("could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id())
                                           + ": failed to get Boolean function at output pin '" + pin->get_name() + "'");
                            }

                            const std::vector<std::string> internal_input_vars = utils::to_vector(bf_interal.get_variable_names());
                            input_vars.insert(input_vars.end(), internal_input_vars.begin(), internal_input_vars.end());

                            if (auto substituted = bf.substitute(var, bf_interal); substituted.is_error())
                            {
                                return ERR_APPEND(substituted.get_error(),
                                                  "could not get Boolean function of gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to substitute variable '"
                                                      + var + "' with another Boolean function");
                            }
                            else
                            {
                                bf = substituted.get();
                            }
                        }
                    }

                    cache.insert({{gate->get_id(), output_pin}, bf});
                    return OK(bf);
                }
            }

            Result<std::monostate> subgraph_function_bfs(Net* n,
                                                         BooleanFunction& current,
                                                         std::vector<Net*> stack,
                                                         const std::vector<const Gate*>& subgraph_gates,
                                                         std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
            {
                if (n->get_num_of_sources() > 1)
                {
                    return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": number of sources is greater than 1");
                }
                else if (n->get_num_of_sources() == 0)
                {
                    return OK({});
                }

                if (auto it = std::find(stack.begin(), stack.end(), n); it != stack.end())
                {
                    return ERR("could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": subgraph contains a cyclic dependency: "
                               + utils::join(" -> ", it, stack.end(), [](auto nlog) { return nlog->get_name() + " (ID: " + std::to_string(nlog->get_id()) + ")"; }) + " -> " + n->get_name()
                               + " (ID: " + std::to_string(n->get_id()) + ")");
                }

                stack.push_back(n);

                Gate* src_gate         = n->get_sources()[0]->get_gate();
                const GatePin* src_pin = n->get_sources()[0]->get_pin();

                if (std::find(subgraph_gates.begin(), subgraph_gates.end(), src_gate) != subgraph_gates.end())
                {
                    if (auto func = get_function_of_gate(src_gate, src_pin, cache); func.is_error())
                    {
                        return ERR_APPEND(func.get_error(),
                                          "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()) + ": failed to get Boolean function of gate '"
                                              + src_gate->get_name() + "' with ID " + std::to_string(src_gate->get_id()));
                    }
                    else
                    {
                        if (auto substitution = current.substitute("net_" + std::to_string(n->get_id()), func.get()); substitution.is_error())
                        {
                            return ERR_APPEND(substitution.get_error(),
                                              "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id())
                                                  + ": failed to substitute net with Boolean function");
                        }
                        else
                        {
                            current = substitution.get();
                        }
                    }

                    for (Net* sn : src_gate->get_fan_in_nets())
                    {
                        if (auto res = subgraph_function_bfs(sn, current, stack, subgraph_gates, cache); res.is_error())
                        {
                            return ERR_APPEND(res.get_error(), "could not get subgraph function of net '" + n->get_name() + "' with ID " + std::to_string(n->get_id()));
                        }
                    }
                }

                return OK({});
            }
        }    // namespace

        Result<BooleanFunction> get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
        {
            /* check validity of subgraph_gates */
            if (subgraph_gates.empty())
            {
                return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": subgraph contains no gates");
            }
            else if (std::any_of(subgraph_gates.begin(), subgraph_gates.end(), [](const Gate* g) { return g == nullptr; }))
            {
                return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": subgraph contains a gate that is a 'nullptr'");
            }
            else if (net == nullptr)
            {
                return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net is a 'nullptr'");
            }
            else if (net->get_num_of_sources() > 1)
            {
                return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has more than one source");
            }
            else if (net->is_global_input_net())
            {
                return OK(BooleanFunction::Var("net_" + std::to_string(net->get_id())));
            }
            else if (net->get_num_of_sources() == 0)
            {
                return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has no sources");
            }

            const Gate* start_gate = net->get_sources()[0]->get_gate();
            if (std::find(subgraph_gates.begin(), subgraph_gates.end(), start_gate) == subgraph_gates.end())
            {
                return OK(BooleanFunction::Var("net_" + std::to_string(net->get_id())));
            }

            const GatePin* src_pin = net->get_sources()[0]->get_pin();

            BooleanFunction result;
            if (auto func = get_function_of_gate(start_gate, src_pin, cache); func.is_error())
            {
                return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to get function of start gate '"
                           + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id()));
            }
            else
            {
                result = func.get();
            }

            for (Net* n : start_gate->get_fan_in_nets())
            {
                if (auto res = subgraph_function_bfs(n, result, {}, subgraph_gates, cache); res.is_error())
                {
                    return ERR_APPEND(res.get_error(),
                                      "could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to get function at net '" + n->get_name()
                                          + "' with ID " + std::to_string(n->get_id()));
                }
            }

            return OK(result);
        }

        Result<BooleanFunction> get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates)
        {
            std::map<std::pair<u32, const GatePin*>, BooleanFunction> cache;
            if (auto res = get_subgraph_function(net, subgraph_gates, cache); res.is_error())
            {
                return ERR(res.get_error());
            }
            else
            {
                return res;
            }
        }

        std::unique_ptr<Netlist> copy_netlist(const Netlist* nl)
        {
            if (auto res = nl->copy(); res.is_error())
            {
                log_error("netlist_utils", "error encountered while copying netlist:\n{}", res.get_error().get());
                return nullptr;
            }
            else
            {
                return res.get();
            }
        }

        std::pair<std::map<u32, Gate*>, std::vector<std::vector<int>>> get_ff_dependency_matrix(const Netlist* nl)
        {
            std::map<u32, Gate*> matrix_id_to_gate;
            std::map<Gate*, u32> gate_to_matrix_id;
            std::vector<std::vector<int>> matrix;

            u32 matrix_gates = 0;
            for (const auto& gate : nl->get_gates())
            {
                if (!gate->get_type()->has_property(GateTypeProperty::ff))
                {
                    continue;
                }
                gate_to_matrix_id[gate]         = matrix_gates;
                matrix_id_to_gate[matrix_gates] = gate;
                matrix_gates++;
            }

            for (const auto& [id, gate] : matrix_id_to_gate)
            {
                std::vector<int> line_of_matrix;

                std::set<u32> gates_to_add;
                for (const auto& pred_gate : netlist_utils::get_next_sequential_gates(gate, false))
                {
                    gates_to_add.insert(gate_to_matrix_id[pred_gate]);
                }

                for (u32 i = 0; i < matrix_gates; i++)
                {
                    if (gates_to_add.find(i) != gates_to_add.end())
                    {
                        line_of_matrix.push_back(1);
                    }
                    else
                    {
                        line_of_matrix.push_back(0);
                    }
                }
                matrix.push_back(line_of_matrix);
            }

            return std::make_pair(matrix_id_to_gate, matrix);
        }

        std::unique_ptr<Netlist> get_partial_netlist(const Netlist* nl, const std::vector<const Gate*>& subgraph_gates)
        {
            std::unique_ptr<Netlist> c_netlist = netlist_factory::create_netlist(nl->get_gate_library());
            c_netlist->enable_automatic_net_checks(false);

            // manager, netlist_id, and top_module are set in the constructor

            // get all relevant nets
            std::set<Net*> all_nets_to_copy;
            for (const Gate* gate : nl->get_gates())
            {
                for (const auto& input_net_of_gate : gate->get_fan_in_nets())
                {
                    all_nets_to_copy.insert(input_net_of_gate);
                }
                for (const auto& output_net_gate : gate->get_fan_out_nets())
                {
                    all_nets_to_copy.insert(output_net_gate);
                }
            }

            // copy nets
            for (const Net* net : all_nets_to_copy)
            {
                Net* new_net = c_netlist->create_net(net->get_id(), net->get_name());
                new_net->set_data_map(net->get_data_map());
            }

            // copy gates
            for (const Gate* gate : subgraph_gates)
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

                // mark gnd and vcc gates
                if (gate->is_gnd_gate())
                {
                    c_netlist->mark_gnd_gate(new_gate);
                }
                if (gate->is_vcc_gate())
                {
                    c_netlist->mark_vcc_gate(new_gate);
                }
            }

            Module* top_module   = nl->get_top_module();
            Module* c_top_module = c_netlist->get_top_module();

            for (Net* c_net : c_netlist->get_nets())
            {
                Net* net = nl->get_net_by_id(c_net->get_id());

                // mark new global inputs
                if (c_net->get_num_of_sources() == 0)
                {
                    if (net->get_num_of_sources() != 0 || net->is_global_input_net())
                    {
                        c_netlist->mark_global_input_net(c_net);
                    }
                }

                // mark nets that had a destination previously but now dont as global outputs
                if (c_net->get_num_of_destinations() == 0)
                {
                    if (net->get_num_of_destinations() != 0 || net->is_global_output_net())
                    {
                        c_netlist->mark_global_output_net(c_net);
                    }
                }
            }

            // update input and output nets
            c_top_module->update_nets();

            // create module pins for top module
            for (Net* c_input_net : c_top_module->get_input_nets())
            {
                // either use existing name of pin or generate new one
                if (auto pin = top_module->get_pin_by_net(nl->get_net_by_id(c_input_net->get_id())); pin != nullptr)
                {
                    if (auto res = c_top_module->create_pin(pin->get_name(), c_input_net, pin->get_type()); res.is_error())
                    {
                        log_error("netlist_utils", "error encountered while copying partial netlist:\n{}", res.get_error().get());
                        return nullptr;
                    }
                }
                else
                {
                    if (auto res = c_top_module->create_pin(c_input_net->get_name(), c_input_net); res.is_error())
                    {
                        log_error("netlist_utils", "error encountered while copying partial netlist:\n{}", res.get_error().get());
                        return nullptr;
                    }
                }
            }
            for (Net* c_output_net : c_top_module->get_output_nets())
            {
                // either use existing name of pin or generate new one
                if (auto pin = top_module->get_pin_by_net(nl->get_net_by_id(c_output_net->get_id())); pin != nullptr)
                {
                    if (auto res = c_top_module->create_pin(pin->get_name(), c_output_net, pin->get_type()); res.is_error())
                    {
                        log_error("netlist_utils", "error encountered while copying partial netlist:\n{}", res.get_error().get());
                        return nullptr;
                    }
                }
                else
                {
                    if (auto res = c_top_module->create_pin(c_output_net->get_name(), c_output_net); res.is_error())
                    {
                        log_error("netlist_utils", "error encountered while copying partial netlist:\n{}", res.get_error().get());
                        return nullptr;
                    }
                }
            }

            // copy some meta data
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

            c_netlist->enable_automatic_net_checks(true);

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
                        if (nets_handled.find(n) != nets_handled.end())
                        {
                            continue;
                        }
                        nets_handled.insert(n);

                        for (const Endpoint* ep : get_successors ? n->get_destinations() : n->get_sources())
                        {
                            Gate* g1 = ep->get_gate();
                            if (gats_handled.find(g1) != gats_handled.end())
                            {
                                continue;    // already handled
                            }
                            gats_handled.insert(g1);
                            if (!filter || filter(g1))
                            {
                                v1.push_back(g1);
                                retval.push_back(g1);
                            }
                        }
                    }
                }
                if (v1.empty())
                {
                    break;
                }
                v0 = v1;
            }
            return retval;
        }

        std::vector<Gate*> get_next_gates(const Net* net, bool get_successors, int depth, const std::function<bool(const Gate*)>& filter)
        {
            std::vector<Gate*> retval;
            std::unordered_map<u32, std::vector<Gate*>> cache;
            std::vector<const Gate*> v0;
            std::unordered_set<const Gate*> gates_handled;
            std::unordered_set<const Net*> nets_handled;
            for (const Endpoint* ep : (get_successors ? net->get_destinations() : net->get_sources()))
            {
                Gate* g = ep->get_gate();
                if (!filter || filter(g))
                {
                    v0.push_back(g);
                    gates_handled.insert(g);
                    nets_handled.insert(net);
                    retval.push_back(g);
                }
            }

            for (int round = 1; depth == 0 || round < depth; round++)
            {
                std::vector<const Gate*> v1;
                for (const Gate* g0 : v0)
                {
                    for (const Net* n : get_successors ? g0->get_fan_out_nets() : g0->get_fan_in_nets())
                    {
                        if (nets_handled.find(n) != nets_handled.end())
                        {
                            continue;
                        }
                        nets_handled.insert(n);

                        for (const Endpoint* ep : get_successors ? n->get_destinations() : n->get_sources())
                        {
                            Gate* g1 = ep->get_gate();
                            if (gates_handled.find(g1) != gates_handled.end())
                            {
                                continue;    // already handled
                            }
                            gates_handled.insert(g1);
                            if (!filter || filter(g1))
                            {
                                v1.push_back(g1);
                                retval.push_back(g1);
                            }
                        }
                    }
                }
                if (v1.empty())
                {
                    break;
                }
                v0 = v1;
            }
            return retval;
        }

        std::vector<Gate*> get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions)
        {
            std::vector<Gate*> path_forward = get_shortest_path_internal(start_gate, end_gate);
            if (!search_both_directions)
                return path_forward;
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

        std::vector<Net*> get_nets_at_pins(Gate* gate, std::vector<GatePin*> pins)
        {
            std::vector<Net*> nets;

            for (const auto& pin : pins)
            {
                PinDirection direction = pin->get_direction();
                if (direction == PinDirection::input || direction == PinDirection::inout)
                {
                    if (auto net = gate->get_fan_in_net(pin); net != nullptr)
                    {
                        nets.push_back(net);
                    }
                    else
                    {
                        log_warning("netlist_utils", "could not retrieve fan-in net for pin '{}' of gate '{}' with ID {}.", pin->get_name(), gate->get_name(), gate->get_id());
                    }
                }
                else if (direction == PinDirection::output)
                {
                    if (auto net = gate->get_fan_out_net(pin); net != nullptr)
                    {
                        nets.push_back(net);
                    }
                    else
                    {
                        log_warning("netlist_utils", "could not retrieve fan-out net for pin '{}' of gate '{}' with ID {}.", pin->get_name(), gate->get_name(), gate->get_id());
                    }
                }
            }

            return nets;
        }

        Result<u32> remove_buffers(Netlist* netlist, bool analyze_inputs)
        {
            u32 num_gates = 0;

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
                if (out_endpoint->get_pin()->get_name() != (functions.begin())->first)
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
                            if (auto substitution = func.substitute(ep->get_pin()->get_name(), BooleanFunction::Const(0, 1)); substitution.is_ok())
                            {
                                func = substitution.get();
                            }
                        }
                        else if (sources.front()->get_gate()->is_vcc_gate())
                        {
                            if (auto substitution = func.substitute(ep->get_pin()->get_name(), BooleanFunction::Const(1, 1)); substitution.is_ok())
                            {
                                func = substitution.get();
                            }
                        }
                    }

                    func = func.simplify();
                }

                std::string func_str             = func.to_string();
                std::vector<std::string> in_pins = gt->get_input_pin_names();
                if (std::find(in_pins.begin(), in_pins.end(), func_str) != in_pins.end())
                {
                    Net* out_net = out_endpoint->get_net();

                    // check all input endpoints and ...
                    for (Endpoint* in_endpoint : fan_in)
                    {
                        Net* in_net = in_endpoint->get_net();

                        if (in_endpoint->get_pin()->get_name() == func_str)
                        {
                            // reconnect outputs if the input is passed through the buffer
                            for (Endpoint* dst : out_net->get_destinations())
                            {
                                Gate* dst_gate   = dst->get_gate();
                                GatePin* dst_pin = dst->get_pin();
                                if (!out_net->remove_destination(dst))
                                {
                                    return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from output net '"
                                               + out_net->get_name() + "' with ID " + std::to_string(out_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                               + std::to_string(gate->get_id()));
                                }
                                if (!in_net->add_destination(dst_gate, dst_pin))
                                {
                                    return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to add destination to input net '"
                                               + in_net->get_name() + "' with ID " + std::to_string(in_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                               + std::to_string(gate->get_id()));
                                }
                            }
                        }
                        else
                        {
                            // remove the input endpoint otherwise
                            if (!in_net->remove_destination(gate, in_endpoint->get_pin()))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from input net '"
                                           + in_net->get_name() + "' with ID " + std::to_string(in_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                           + std::to_string(gate->get_id()));
                            }
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

                    const std::vector<Gate*>& gnd_gates = netlist->get_gnd_gates();
                    const std::vector<Gate*>& vcc_gates = netlist->get_vcc_gates();
                    if (gnd_gates.empty() || vcc_gates.empty())
                    {
                        continue;
                    }
                    Net* gnd_net = gnd_gates.front()->get_fan_out_nets().front();
                    Net* vcc_net = vcc_gates.front()->get_fan_out_nets().front();

                    for (Endpoint* in_endpoint : fan_in)
                    {
                        Net* in_net = in_endpoint->get_net();

                        // remove the input endpoint otherwise
                        if (!in_net->remove_destination(gate, in_endpoint->get_pin()))
                        {
                            return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from input net '"
                                       + in_net->get_name() + "' with ID " + std::to_string(in_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()));
                        }
                    }
                    if (func_str == "0")
                    {
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate   = dst->get_gate();
                            GatePin* dst_pin = dst->get_pin();
                            if (!out_net->remove_destination(dst))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from output net '"
                                           + out_net->get_name() + "' with ID " + std::to_string(out_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                           + std::to_string(gate->get_id()));
                            }
                            if (!gnd_net->add_destination(dst_gate, dst_pin))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to add destination to GND net '"
                                           + gnd_net->get_name() + "' with ID " + std::to_string(gnd_net->get_id()));
                            }
                        }
                    }
                    else if (func_str == "1")
                    {
                        for (Endpoint* dst : out_net->get_destinations())
                        {
                            Gate* dst_gate   = dst->get_gate();
                            GatePin* dst_pin = dst->get_pin();
                            if (!out_net->remove_destination(dst))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to remove destination from output net '"
                                           + out_net->get_name() + "' with ID " + std::to_string(out_net->get_id()) + " of buffer gate '" + gate->get_name() + "' with ID "
                                           + std::to_string(gate->get_id()));
                            }
                            if (!vcc_net->add_destination(dst_gate, dst_pin))
                            {
                                return ERR("could not completely remove buffers from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to add destination to VCC net '"
                                           + gnd_net->get_name() + "' with ID " + std::to_string(gnd_net->get_id()));
                            }
                        }
                    }

                    // delete output net and buffer gate
                    netlist->delete_net(out_net);
                    netlist->delete_gate(gate);
                    num_gates++;
                }
            }

            return OK(num_gates);
        }

        Result<u32> remove_unused_lut_endpoints(Netlist* netlist)
        {
            u32 num_eps = 0;

            // net connected to GND
            const std::vector<Gate*>& gnd_gates = netlist->get_gnd_gates();
            if (gnd_gates.empty())
            {
                return ERR("could not completely remove unused LUT endpoints from netlist with ID " + std::to_string(netlist->get_id()) + ": no GND net available within netlist");
            }
            Net* gnd_net = gnd_gates.front()->get_fan_out_nets().front();

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

                auto active_pins = functions.begin()->second.get_variable_names();

                // if there are more fan-in nets than there are active pins, we need to get rid of some nets
                if (fan_in.size() > active_pins.size())
                {
                    for (const auto& ep : fan_in)
                    {
                        if (std::find(active_pins.begin(), active_pins.end(), ep->get_pin()->get_name()) == active_pins.end())
                        {
                            num_eps++;
                            GatePin* pin = ep->get_pin();
                            if (!ep->get_net()->remove_destination(gate, pin))
                            {
                                return ERR("could not completely remove unused LUT endpoints from netlist with ID " + std::to_string(netlist->get_id())
                                           + ": failed to remove inactive endpoint from gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()));
                            }
                            if (!gnd_net->add_destination(gate, pin))
                            {
                                return ERR("could not completely remove unused LUT endpoints from netlist with ID " + std::to_string(netlist->get_id()) + ": failed to connect inactive input of gate '"
                                           + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + " to GND net");
                            }
                        }
                    }
                }
            }

            return OK(num_eps);
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

        Result<std::monostate> replace_gate(Gate* gate, GateType* target_type, std::map<GatePin*, GatePin*> pin_map)
        {
            if (gate == nullptr)
            {
                return ERR("could not replace gate: gate is a 'nullptr'");
            }
            if (target_type == nullptr)
            {
                return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": target gate type is a 'nullptr'");
            }

            Netlist* netlist                  = gate->get_netlist();
            u32 gate_id                       = gate->get_id();
            std::string gate_name             = gate->get_name();
            std::pair<i32, i32> gate_location = gate->get_location();
            std::vector<Endpoint*> fan_in     = gate->get_fan_in_endpoints();
            std::vector<Endpoint*> fan_out    = gate->get_fan_out_endpoints();
            Module* gate_module               = gate->get_module();
            Grouping* gate_grouping           = gate->get_grouping();
            auto gate_data                    = gate->get_data_map();

            std::map<GatePin*, Net*> in_nets;
            std::map<GatePin*, Net*> out_nets;

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
            if (netlist->delete_gate(gate) == false)
            {
                return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to delete old gate of type '" + gate->get_type()->get_name()
                           + "'");
            }
            // create new gate
            Gate* new_gate = netlist->create_gate(gate_id, target_type, gate_name, gate_location.first, gate_location.second);
            if (new_gate == nullptr)
            {
                return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to create replacement gate of type '" + target_type->get_name()
                           + "'");
            }

            // reconnect nets
            for (auto [in_pin, in_net] : in_nets)
            {
                if (!in_net->add_destination(new_gate, in_pin))
                {
                    return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to reconnect input net '" + in_net->get_name() + "' with ID "
                               + std::to_string(in_net->get_id()) + " to pin '" + in_pin->get_name() + "' of the replacement gate");
                }
            }

            for (const auto& [out_pin, out_net] : out_nets)
            {
                if (!out_net->add_source(new_gate, out_pin))
                {
                    return ERR("could not replace gate '" + gate->get_name() + "' with ID " + std::to_string(gate->get_id()) + ": failed to reconnect output net '" + out_net->get_name() + "' with ID "
                               + std::to_string(out_net->get_id()) + " to pin '" + out_pin->get_name() + "' of the replacement gate");
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

            return OK({});
        }

        Result<std::vector<Gate*>>
            get_gate_chain(Gate* start_gate, const std::vector<const GatePin*>& input_pins, const std::vector<const GatePin*>& output_pins, const std::function<bool(const Gate*)>& filter)
        {
            if (start_gate == nullptr)
            {
                return ERR("could not detect gate chain at start gate: start gate is a 'nullptr'");
            }

            // check filter on start gate
            if (filter && !filter(start_gate))
            {
                return ERR("could not detect gate chain at start gate '" + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id())
                           + ": filter evaluates to 'false' for start gate");
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
                std::vector<Endpoint*> successors = current_gate->get_successors([input_pins, output_pins, target_type, filter](const GatePin* ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (output_pins.empty() || std::find(output_pins.begin(), output_pins.end(), ep_pin) != output_pins.end())
                        {
                            if (input_pins.empty() || std::find(input_pins.begin(), input_pins.end(), ep->get_pin()) != input_pins.end())
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
                std::vector<Endpoint*> predecessors = current_gate->get_predecessors([input_pins, output_pins, target_type, filter](const GatePin* ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (input_pins.empty() || std::find(input_pins.begin(), input_pins.end(), ep_pin) != input_pins.end())
                        {
                            if (output_pins.empty() || std::find(output_pins.begin(), output_pins.end(), ep->get_pin()) != output_pins.end())
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

            return OK(std::vector<Gate*>(gate_chain.begin(), gate_chain.end()));
        }

        Result<std::vector<Gate*>> get_complex_gate_chain(Gate* start_gate,
                                                          const std::vector<GateType*>& chain_types,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& input_pins,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& output_pins,
                                                          const std::function<bool(const Gate*)>& filter)
        {
            if (start_gate == nullptr)
            {
                return ERR("could not detect gate chain at start gate: start gate is a 'nullptr'");
            }
            if (chain_types.size() < 2)
            {
                return ERR("could not detect gate chain at start gate: 'chain_types' comprises less than two target gate types");
            }
            if (start_gate->get_type() != chain_types.at(0))
            {
                return ERR("could not detect gate chain at start gate '" + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id()) + ": start gate is not of type '"
                           + chain_types.front()->get_name() + "'");
            }
            if (filter && !filter(start_gate))
            {
                return ERR("could not detect gate chain at start gate '" + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id())
                           + ": filter evaluates to 'false' for start gate");
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
                GateType* target_type                      = chain_types.at(current_index);
                const std::vector<const GatePin*>& inputs  = input_pins.at(target_type);
                const std::vector<const GatePin*>& outputs = output_pins.at(chain_types.at(last_index));
                std::vector<Endpoint*> successors          = current_gate->get_successors([target_type, inputs, outputs, filter](const GatePin* ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (outputs.empty() || std::find(outputs.begin(), outputs.end(), ep_pin) != outputs.end())
                        {
                            if (inputs.empty() || std::find(inputs.begin(), inputs.end(), ep->get_pin()) != inputs.end())
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
                GateType* target_type                      = chain_types.at(current_index);
                const std::vector<const GatePin*>& inputs  = input_pins.at(chain_types.at(last_index));
                const std::vector<const GatePin*>& outputs = output_pins.at(target_type);
                std::vector<Endpoint*> predecessors        = current_gate->get_predecessors([target_type, inputs, outputs, filter](const GatePin* ep_pin, Endpoint* ep) {
                    if (ep->get_gate()->get_type() == target_type)
                    {
                        if (inputs.empty() || std::find(inputs.begin(), inputs.end(), ep_pin) != inputs.end())
                        {
                            if (outputs.empty() || std::find(outputs.begin(), outputs.end(), ep->get_pin()) != outputs.end())
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

            return OK(std::vector<Gate*>(gate_chain.begin(), gate_chain.end()));
        }
    }    // namespace netlist_utils
}    // namespace hal
