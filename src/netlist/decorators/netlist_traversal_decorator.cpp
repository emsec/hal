#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <deque>

namespace hal
{
    std::vector<Gate*> NetlistTraversalDecorator::get_next_gates(const Gate* gate, bool get_successors, int depth, const std::function<bool(const Gate*)>& filter)
    {
        std::vector<Gate*> retval;
        std::vector<const Gate*> v0;
        std::unordered_set<const Gate*> gates_handled;
        std::unordered_set<const Net*> nets_handled;

        v0.push_back(gate);
        gates_handled.insert(gate);

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

    std::vector<Gate*> NetlistTraversalDecorator::get_next_gates(const Net* net, bool get_successors, int depth, const std::function<bool(const Gate*)>& filter)
    {
        std::vector<Gate*> retval;

        for (const Endpoint* ep : (get_successors ? net->get_destinations() : net->get_sources()))
        {
            Gate* g = ep->get_gate();
            if (!filter || filter(g))
            {
                const auto next = get_next_gates(g, get_successors, depth, filter);
                retval.insert(retval.end(), next.begin(), next.end());
            }
        }

        return retval;
    }

    namespace
    {
        std::vector<const Gate*> get_shortest_path_internal(const Gate* start_gate, const Gate* end_gate)
        {
            if (start_gate == end_gate)
            {
                return std::vector<const Gate*>();
            }
            std::vector<const Gate*> v0;
            v0.push_back(start_gate);
            std::unordered_map<const Gate*, const Gate*> originMap;
            for (;;)
            {
                std::vector<const Gate*> v1;
                for (const Gate* g0 : v0)
                {
                    for (const Gate* g1 : NetlistTraversalDecorator(*start_gate->get_netlist()).get_next_gates(g0, true, 1))
                    {
                        if (originMap.find(g1) != originMap.end())
                        {
                            continue;    // already routed to
                        }
                        v1.push_back(g1);
                        originMap[g1] = g0;
                        if (g1 == end_gate)
                        {
                            // heureka!
                            std::vector<const Gate*> retval;
                            const Gate* g = end_gate;
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
                {
                    break;
                }
                v0 = v1;
            }
            return std::vector<const Gate*>();
        }

        std::vector<Gate*> get_shortest_path_internal(Gate* start_gate, Gate* end_gate)
        {
            if (start_gate == end_gate)
            {
                return std::vector<Gate*>();
            }
            std::vector<Gate*> v0;
            v0.push_back(start_gate);
            std::unordered_map<Gate*, Gate*> originMap;
            for (;;)
            {
                std::vector<Gate*> v1;
                for (Gate* g0 : v0)
                {
                    for (Gate* g1 : NetlistTraversalDecorator(*start_gate->get_netlist()).get_next_gates(g0, true, 1))
                    {
                        if (originMap.find(g1) != originMap.end())
                        {
                            continue;    // already routed to
                        }
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
                {
                    break;
                }
                v0 = v1;
            }
            return std::vector<Gate*>();
        }
    }    // namespace

    std::vector<const Gate*> NetlistTraversalDecorator::get_shortest_path(const Gate* start_gate, const Gate* end_gate, bool search_both_directions)
    {
        std::vector<const Gate*> path_forward = get_shortest_path_internal(start_gate, end_gate);
        if (!search_both_directions)
        {
            return path_forward;
        }
        std::vector<const Gate*> path_reverse = get_shortest_path_internal(end_gate, start_gate);

        if (((path_forward.size() < path_reverse.size()) || path_reverse.empty()) && !path_forward.empty())
        {
            return path_forward;
        }

        if (((path_reverse.size() < path_forward.size()) || path_forward.empty()) && !path_reverse.empty())
        {
            return path_reverse;
        }

        return path_forward;
    }

    std::vector<Gate*> NetlistTraversalDecorator::get_shortest_path(Gate* start_gate, Gate* end_gate, bool search_both_directions)
    {
        std::vector<Gate*> path_forward = get_shortest_path_internal(start_gate, end_gate);
        if (!search_both_directions)
        {
            return path_forward;
        }
        std::vector<Gate*> path_reverse = get_shortest_path_internal(end_gate, start_gate);
        return (path_reverse.size() < path_forward.size()) ? path_reverse : path_forward;
    }

    namespace
    {
        std::vector<Gate*> get_next_sequential_gates_internal(const Net* start_net,
                                                              bool forward,
                                                              std::unordered_set<u32>& seen_nets,
                                                              std::unordered_set<u32>& seen_gates,
                                                              std::unordered_map<u32, std::vector<Gate*>>& cache,
                                                              u32 depth)
        {
            if (auto it = cache.find(start_net->get_id()); it != cache.end())
            {
                return it->second;
            }

            if (seen_nets.find(start_net->get_id()) != seen_nets.end())
            {
                return {};
            }
            seen_nets.insert(start_net->get_id());

            std::vector<Gate*> found_ffs;

            for (auto endpoint : forward ? start_net->get_destinations() : start_net->get_sources())
            {
                auto next_gate = endpoint->get_gate();
                if (seen_gates.find(next_gate->get_id()) != seen_gates.end())
                {
                    continue;
                }
                seen_gates.insert(next_gate->get_id());

                if (next_gate->get_type()->has_property(GateTypeProperty::ff))
                {
                    if (depth == 1)
                    {
                        found_ffs.push_back(next_gate);
                    }
                    else
                    {
                        for (auto n : forward ? next_gate->get_fan_out_nets() : next_gate->get_fan_in_nets())
                        {
                            const u32 new_depth = !depth ? 0 : depth - 1;
                            auto next_gates     = get_next_sequential_gates_internal(n, forward, seen_nets, seen_gates, cache, new_depth);
                            found_ffs.insert(found_ffs.end(), next_gates.begin(), next_gates.end());
                        }
                    }
                }
                else
                {
                    for (auto n : forward ? next_gate->get_fan_out_nets() : next_gate->get_fan_in_nets())
                    {
                        auto next_gates = get_next_sequential_gates_internal(n, forward, seen_nets, seen_gates, cache, depth);
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

    std::vector<Gate*> NetlistTraversalDecorator::get_next_sequential_gates(const Gate* gate, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache, const u32 depth)
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

    std::vector<Gate*> NetlistTraversalDecorator::get_next_sequential_gates(const Net* net, bool get_successors, std::unordered_map<u32, std::vector<Gate*>>& cache, const u32 depth)
    {
        std::unordered_set<u32> seen_nets;
        std::unordered_set<u32> seen_gates;
        return get_next_sequential_gates_internal(net, get_successors, seen_nets, seen_gates, cache, depth);
    }

    std::vector<Gate*> NetlistTraversalDecorator::get_next_sequential_gates(const Gate* gate, bool get_successors, const u32 depth)
    {
        std::unordered_map<u32, std::vector<Gate*>> cache;
        return get_next_sequential_gates(gate, get_successors, cache, depth);
    }

    std::vector<Gate*> NetlistTraversalDecorator::get_next_sequential_gates(const Net* net, bool get_successors, const u32 depth)
    {
        std::unordered_map<u32, std::vector<Gate*>> cache;
        return get_next_sequential_gates(net, get_successors, cache, depth);
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

    std::vector<Gate*> NetlistTraversalDecorator::get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache)
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

    std::vector<Gate*> NetlistTraversalDecorator::get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties, std::unordered_map<u32, std::vector<Gate*>>& cache)
    {
        std::unordered_set<u32> seen;
        return get_path_internal(net, get_successors, stop_properties, seen, cache);
    }

    std::vector<Gate*> NetlistTraversalDecorator::get_path(const Gate* gate, bool get_successors, std::set<GateTypeProperty> stop_properties)
    {
        std::unordered_map<u32, std::vector<Gate*>> cache;
        return get_path(gate, get_successors, stop_properties, cache);
    }

    std::vector<Gate*> NetlistTraversalDecorator::get_path(const Net* net, bool get_successors, std::set<GateTypeProperty> stop_properties)
    {
        std::unordered_map<u32, std::vector<Gate*>> cache;
        return get_path(net, get_successors, stop_properties, cache);
    }

    Result<std::vector<Gate*>> NetlistTraversalDecorator::get_gate_chain(Gate* start_gate,
                                                                         const std::vector<const GatePin*>& input_pins,
                                                                         const std::vector<const GatePin*>& output_pins,
                                                                         const std::function<bool(const Gate*)>& filter)
    {
        if (start_gate == nullptr)
        {
            return ERR("could not detect gate chain at start gate: start gate is a 'nullptr'");
        }

        // check filter on start gate
        if (filter && !filter(start_gate))
        {
            return ERR("could not detect gate chain at start gate '" + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id()) + ": filter evaluates to 'false' for start gate");
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

    Result<std::vector<Gate*>> NetlistTraversalDecorator::get_complex_gate_chain(Gate* start_gate,
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
            return ERR("could not detect gate chain at start gate '" + start_gate->get_name() + "' with ID " + std::to_string(start_gate->get_id()) + ": filter evaluates to 'false' for start gate");
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
}    // namespace hal