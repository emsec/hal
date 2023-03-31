#include "hal_core/netlist/netlist_utils.h"

#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
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
        }    // namespace

        Result<BooleanFunction> get_subgraph_function(const Net* net, const std::vector<const Gate*>& subgraph_gates, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
        {
            if (net == nullptr)
            {
                return ERR("could not get subgraph function: net is a 'nullptr'");
            }

            if (auto res = SubgraphNetlistDecorator(*net->get_netlist()).get_subgraph_function(subgraph_gates, net, cache); res.is_ok())
            {
                return res;
            }
            else
            {
                return ERR(res.get_error());
            }
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
            if (auto res = SubgraphNetlistDecorator(*nl).copy_subgraph_netlist(subgraph_gates); res.is_ok())
            {
                return res.get();
            }
            else
            {
                log_error("netlist_utils", "error encountered while copying subgraph netlist:\n{}", res.get_error().get());
                return nullptr;
            }
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
                if (pin == nullptr)
                {
                    log_warning("netlist_utils", "'nullptr' given as pin.");
                    continue;
                }

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
            for (const auto& gate : netlist->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_lut); }))
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
            if (auto res = NetlistModificationDecorator(*(gate->get_netlist())).replace_gate(gate, target_type, pin_map); res.is_ok())
            {
                return OK({});
            }
            else
            {
                return ERR(res.get_error());
            }
        }

        Result<std::monostate> merge_nets(Netlist* netlist, Net* net_in, Net* net_out, const bool in_survives)
        {
            const auto survivor_net      = in_survives ? net_in : net_out;
            const auto net_to_be_deleted = in_survives ? net_out : net_in;

            // saved information
            bool is_global = in_survives ? net_to_be_deleted->is_global_output_net() : net_to_be_deleted->is_global_input_net();
            std::map<Module*, std::tuple<std::string, std::string, u32, PinDirection, PinType>> module_pins;

            // safe all module pin information from the net soon to be deleted
            for (const auto& m : netlist->get_modules())
            {
                if (const auto pin = m->get_pin_by_net(net_to_be_deleted); pin != nullptr)
                {
                    module_pins.insert({m, {pin->get_group().first->get_name(), pin->get_name(), pin->get_group().second, pin->get_direction(), pin->get_type()}});
                }
            }

            if (in_survives)
            {
                // copy destinations to net_in
                for (Endpoint* dst : net_out->get_destinations())
                {
                    Gate* dst_gate   = dst->get_gate();
                    GatePin* dst_pin = dst->get_pin();
                    if (!net_out->remove_destination(dst))
                    {
                        return ERR("unable to fuse net " + net_in->get_name() + " with ID " + std::to_string(net_in->get_id()) + " with net " + net_out->get_name() + " with ID "
                                   + std::to_string(net_out->get_id()) + ": unable to remove destination at gate " + dst_gate->get_name() + " with ID " + std::to_string(dst_gate->get_id())
                                   + " at pin " + dst_pin->get_name());
                    }
                    if (!net_in->add_destination(dst_gate, dst_pin))
                    {
                        return ERR("unable to fuse net " + net_in->get_name() + " with ID " + std::to_string(net_in->get_id()) + " with net " + net_out->get_name() + " with ID "
                                   + std::to_string(net_out->get_id()) + ": unable to add destination at gate " + dst_gate->get_name() + " with ID " + std::to_string(dst_gate->get_id()) + " at pin "
                                   + dst_pin->get_name());
                    }
                }
            }
            else
            {
                // copy sources to net_out
                for (Endpoint* src : net_in->get_sources())
                {
                    Gate* src_gate   = src->get_gate();
                    GatePin* src_pin = src->get_pin();
                    if (!net_in->remove_source(src))
                    {
                        return ERR("unable to fuse net " + net_in->get_name() + " with ID " + std::to_string(net_in->get_id()) + " with net " + net_out->get_name() + " with ID "
                                   + std::to_string(net_out->get_id()) + ": unable to remove source at gate " + src_gate->get_name() + " with ID " + std::to_string(src_gate->get_id()) + " at pin "
                                   + src_pin->get_name());
                    }
                    if (!net_out->add_source(src_gate, src_pin))
                    {
                        return ERR("unable to fuse net " + net_in->get_name() + " with ID " + std::to_string(net_in->get_id()) + " with net " + net_out->get_name() + " with ID "
                                   + std::to_string(net_out->get_id()) + ": unable to add source at gate " + src_gate->get_name() + " with ID " + std::to_string(src_gate->get_id()) + " at pin "
                                   + src_pin->get_name());
                    }
                }
            }

            // delete net
            if (!netlist->delete_net(net_to_be_deleted))
            {
                return ERR("unable to fuse net " + net_in->get_name() + " with ID " + std::to_string(net_in->get_id()) + " with net " + net_out->get_name() + " with ID "
                           + std::to_string(net_out->get_id()) + ": unable to delete net " + net_to_be_deleted->get_name() + " with ID " + std::to_string(net_to_be_deleted->get_id()));
            }

            // transfer global information to survivor net
            if (is_global)
            {
                if (in_survives ? !netlist->mark_global_output_net(survivor_net) : !netlist->mark_global_input_net(survivor_net))
                {
                    return ERR("unable to fuse net " + net_in->get_name() + " with ID " + std::to_string(net_in->get_id()) + " with net " + net_out->get_name() + " with ID "
                               + std::to_string(net_out->get_id()) + ": unable to mark net as global " + survivor_net->get_name() + " with ID " + std::to_string(survivor_net->get_id()));
                }
            }

            // transfer module information to survivor net
            for (auto& [m, pin_info] : module_pins)
            {
                const std::string pingroup_name = std::get<0>(pin_info);
                const std::string pin_name      = std::get<1>(pin_info);
                const PinType pin_type          = std::get<4>(pin_info);

                // get pin that was created automatically when connecting the net to a gate inside the module
                if (auto pin = m->get_pin_by_net(survivor_net); pin != nullptr)
                {
                    pin->set_name(pin_name);
                    pin->set_type(pin_type);

                    // remove pin from current pin group
                    auto current_pin_group = pin->get_group().first;
                    current_pin_group->remove_pin(pin).get();
                    if (current_pin_group->get_pins().empty())
                    {
                        m->delete_pin_group(current_pin_group);
                    }

                    // check for existing pingroup otherwise create it
                    auto pin_groups = m->get_pin_groups([pingroup_name](const auto& pg) { return pg->get_name() == pingroup_name; });
                    PinGroup<ModulePin>* pin_group;
                    if (pin_groups.empty())
                    {
                        pin_group = m->create_pin_group(pingroup_name, {}, PinDirection::none, pin_type).get();
                    }
                    else
                    {
                        pin_group = pin_groups.front();
                    }

                    pin_group->assign_pin(pin).get();
                    pin_group->move_pin(pin, std::get<2>(pin_info)).get();
                }
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

                // check all eligible successors of current gate
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
