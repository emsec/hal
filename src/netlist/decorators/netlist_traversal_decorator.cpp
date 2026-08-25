#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

#include "hal_core/netlist/module.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetlistTraversalDecorator::NetlistTraversalDecorator(const Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_gates(const Net* net,
                                                                 TraversalDirection direction,
                                                                 const std::function<bool(const Gate*)>& match,
                                                                 TraversalStop stop,
                                                                 u32 max_depth,
                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (net == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(net))
        {
            return ERR("net does not belong to netlist");
        }

        if (!match)
        {
            return ERR("no match condition specified");
        }

        if (direction == TraversalDirection::both)
        {
            auto res_forward = get_gates(net, TraversalDirection::forward, match, stop, max_depth, exit_endpoint_filter, entry_endpoint_filter);
            if (res_forward.is_error())
            {
                return res_forward;
            }

            auto res_backward = get_gates(net, TraversalDirection::backward, match, stop, max_depth, exit_endpoint_filter, entry_endpoint_filter);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            auto gates = res_forward.get();
            gates.merge(res_backward.get());
            return OK(gates);
        }

        const bool successors = (direction == TraversalDirection::forward);

        std::set<Gate*> res;
        std::vector<const Net*> stack     = {net};
        std::vector<const Net*> previous  = {};
        std::unordered_set<const Net*> visited;

        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            const u32 current_depth = previous.size() + 1;
            if (max_depth != 0 && current_depth > max_depth)
            {
                stack.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (const auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, current_depth))
                {
                    continue;
                }

                auto* gate = entry_ep->get_gate();

                const bool matches = match(gate);
                if (matches)
                {
                    res.insert(gate);
                }

                // Where the walk halts. A gate that ends it is still reported if it matched, it is only
                // not traversed through.
                if ((matches && stop == TraversalStop::at_match) || (!matches && stop == TraversalStop::at_mismatch))
                {
                    continue;
                }

                if (max_depth != 0 && current_depth == max_depth)
                {
                    continue;
                }

                for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
                {
                    if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, current_depth))
                    {
                        continue;
                    }

                    const Net* exit_net = exit_ep->get_net();
                    if (visited.find(exit_net) == visited.end())
                    {
                        stack.push_back(exit_net);
                        added = true;
                    }
                }
            }

            if (added)
            {
                previous.push_back(current);
            }
            else
            {
                stack.pop_back();
            }
        }

        return OK(res);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_gates(const Gate* gate,
                                                                 TraversalDirection direction,
                                                                 const std::function<bool(const Gate*)>& match,
                                                                 TraversalStop stop,
                                                                 u32 max_depth,
                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("gate does not belong to netlist");
        }

        if (!match)
        {
            return ERR("no match condition specified");
        }

        // For both, the fan-out nets are walked forward and the fan-in nets backward, each purely. Passing
        // `both` down to the per-net walks instead would also walk *forward* from a fan-in net, which runs
        // into the cones of sibling gates sharing that input -- gates that are neither ancestors nor
        // descendants of this one.
        std::set<Gate*> res;
        if (direction == TraversalDirection::forward || direction == TraversalDirection::both)
        {
            for (const auto* exit_ep : gate->get_fan_out_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 1))
                {
                    continue;
                }

                auto res_net = get_gates(exit_ep->get_net(), TraversalDirection::forward, match, stop, max_depth, exit_endpoint_filter, entry_endpoint_filter);
                if (res_net.is_error())
                {
                    return ERR_APPEND(res_net.get_error(), "cannot traverse from gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
                }
                res.merge(res_net.get());
            }
        }

        if (direction == TraversalDirection::backward || direction == TraversalDirection::both)
        {
            for (const auto* exit_ep : gate->get_fan_in_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 1))
                {
                    continue;
                }

                auto res_net = get_gates(exit_ep->get_net(), TraversalDirection::backward, match, stop, max_depth, exit_endpoint_filter, entry_endpoint_filter);
                if (res_net.is_error())
                {
                    return ERR_APPEND(res_net.get_error(), "cannot traverse from gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
                }
                res.merge(res_net.get());
            }
        }

        return OK(res);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_matching_gates(const Net* net,
                                                                               bool successors,
                                                                               const std::function<bool(const Gate*)>& target_gate_filter,
                                                                               bool continue_on_match,
                                                                               const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                               const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        return get_gates(net, successors ? TraversalDirection::forward : TraversalDirection::backward, target_gate_filter,
                         continue_on_match ? TraversalStop::never : TraversalStop::at_match, 0, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_matching_gates(const Gate* gate,
                                                                               bool successors,
                                                                               const std::function<bool(const Gate*)>& target_gate_filter,
                                                                               bool continue_on_match,
                                                                               const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                               const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("net does not belong to netlist");
        }

        std::set<Gate*> res;
        for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
            {
                continue;
            }

            const auto* exit_net = exit_ep->get_net();
            const auto next_res  = this->get_next_matching_gates(exit_net, successors, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_matching_gates_until(const Net* net,
                                                                                     bool successors,
                                                                                     const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                     bool continue_on_mismatch,
                                                                                     const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                     const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        return get_gates(net, successors ? TraversalDirection::forward : TraversalDirection::backward, target_gate_filter,
                         continue_on_mismatch ? TraversalStop::never : TraversalStop::at_mismatch, 0, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_matching_gates_until(const Gate* gate,
                                                                                     bool successors,
                                                                                     const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                     bool continue_on_mismatch,
                                                                                     const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                     const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("net does not belong to netlist");
        }

        std::set<Gate*> res;
        for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
            {
                continue;
            }

            const auto next_res = this->get_next_matching_gates_until(exit_ep->get_net(), successors, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_matching_gates_until_depth(const Net* net, bool successors, u32 max_depth, const std::function<bool(const Gate*)>& target_gate_filter) const
    {
        const auto match = (target_gate_filter == nullptr) ? std::function<bool(const Gate*)>([](const Gate*) { return true; }) : target_gate_filter;
        return get_gates(net, successors ? TraversalDirection::forward : TraversalDirection::backward, match, TraversalStop::never, max_depth);
    }

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_matching_gates_until_depth(const Gate* gate, bool successors, u32 max_depth, const std::function<bool(const Gate*)>& target_gate_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("net does not belong to netlist");
        }

        std::set<Gate*> res;
        for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            const auto next_res = this->get_next_matching_gates_until_depth(exit_ep->get_net(), successors, max_depth, target_gate_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_gates_memoized(const Net* start,
                                                                          bool successors,
                                                                          const std::function<bool(const Gate*)>& match,
                                                                          TraversalStop stop,
                                                                          const std::function<bool(const Endpoint*)>& exit_endpoint_filter,
                                                                          const std::function<bool(const Endpoint*)>& entry_endpoint_filter,
                                                                          std::unordered_map<const Net*, std::set<Gate*>>& store) const
    {
        if (start == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(start))
        {
            return ERR("net does not belong to netlist");
        }

        if (!match)
        {
            return ERR("no match condition specified");
        }

        if (const auto it = store.find(start); it != store.end())
        {
            return OK(it->second);
        }

        // Tarjan's algorithm over the nets, iteratively. `collected` holds, per unpublished net, the
        // gates found next to it plus everything absorbed from already published successors; when a
        // strongly connected component completes, its members share the union of what they collected
        // and are published together.
        struct Frame
        {
            const Net* net;
            std::vector<const Net*> successors;
            size_t next = 0;
        };

        std::unordered_map<const Net*, u32> index;
        std::unordered_map<const Net*, u32> lowlink;
        std::unordered_map<const Net*, std::set<Gate*>> collected;
        std::vector<const Net*> scc_stack;
        std::unordered_set<const Net*> on_stack;
        std::vector<Frame> frames;
        u32 next_index = 0;

        const auto expand = [&](const Net* net) {
            index[net] = lowlink[net] = next_index++;
            scc_stack.push_back(net);
            on_stack.insert(net);

            Frame frame{net, {}, 0};
            for (const auto* entry_ep : successors ? net->get_destinations() : net->get_sources())
            {
                if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep))
                {
                    continue;
                }

                auto* gate         = entry_ep->get_gate();
                const bool matches = match(gate);
                if (matches)
                {
                    collected[net].insert(gate);
                }

                const bool traverse = (stop == TraversalStop::at_match) ? !matches : (stop == TraversalStop::at_mismatch) ? matches : true;
                if (!traverse)
                {
                    continue;
                }

                for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
                {
                    if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep))
                    {
                        continue;
                    }

                    frame.successors.push_back(exit_ep->get_net());
                }
            }
            frames.push_back(std::move(frame));
        };

        expand(start);
        while (!frames.empty())
        {
            Frame& frame = frames.back();
            if (frame.next < frame.successors.size())
            {
                const Net* child = frame.successors.at(frame.next++);
                if (const auto it = store.find(child); it != store.end() && on_stack.find(child) == on_stack.end())
                {
                    // published, by an earlier call or by a component that completed within this one
                    collected[frame.net].insert(it->second.begin(), it->second.end());
                }
                else if (index.find(child) == index.end())
                {
                    expand(child);
                }
                else if (on_stack.find(child) != on_stack.end())
                {
                    lowlink[frame.net] = std::min(lowlink[frame.net], index[child]);
                }
            }
            else
            {
                const Net* net = frame.net;
                frames.pop_back();

                if (lowlink[net] == index[net])
                {
                    // the component is complete: its members share one answer and are published together
                    std::vector<const Net*> members;
                    std::set<Gate*> total;
                    while (true)
                    {
                        const Net* member = scc_stack.back();
                        scc_stack.pop_back();
                        on_stack.erase(member);
                        members.push_back(member);
                        auto& part = collected[member];
                        total.insert(part.begin(), part.end());
                        if (member == net)
                        {
                            break;
                        }
                    }
                    for (const Net* member : members)
                    {
                        store[member] = total;
                    }
                }

                if (!frames.empty())
                {
                    Frame& parent = frames.back();
                    lowlink[parent.net] = std::min(lowlink[parent.net], lowlink[net]);
                    if (const auto it = store.find(net); it != store.end())
                    {
                        collected[parent.net].insert(it->second.begin(), it->second.end());
                    }
                }
            }
        }

        return OK(store[start]);
    }

    // The two traversals below are the memoized walk with their conditions pinned. They exist by name
    // because the questions they answer are asked constantly, and they keep the caller-supplied store
    // so that the Boolean influence plugin can share one across the flip-flops of a netlist.
    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_sequential_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache) const
    {
        // A forbidden pin stops the walk at a sequential gate entered through it, so that a flip-flop
        // reached through its clock, for example, does not count. A combinational gate is traversed
        // regardless of the pin it is entered through, which is what the previous implementation did.
        const auto entry = [&forbidden_pins](const Endpoint* ep) {
            return !(ep->get_gate()->get_type()->has_property(GateTypeProperty::sequential) && forbidden_pins.find(ep->get_pin()->get_type()) != forbidden_pins.end());
        };
        const auto exit  = [&forbidden_pins](const Endpoint* ep) { return forbidden_pins.find(ep->get_pin()->get_type()) == forbidden_pins.end(); };
        const auto match = [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::sequential); };

        std::unordered_map<const Net*, std::set<Gate*>> local_store;
        return get_gates_memoized(net, successors, match, TraversalStop::at_match, exit, entry, (cache != nullptr) ? *cache : local_store);
    }

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_sequential_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("gate does not belong to netlist");
        }

        std::set<Gate*> res;
        std::unordered_map<const Net*, std::set<Gate*>> local_store;
        for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (forbidden_pins.find(exit_ep->get_pin()->get_type()) != forbidden_pins.end())
            {
                continue;
            }

            auto res_net = get_next_sequential_gates(exit_ep->get_net(), successors, forbidden_pins, (cache != nullptr) ? cache : &local_store);
            if (res_net.is_error())
            {
                return ERR_APPEND(res_net.get_error(), "cannot get next sequential gates of gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
            }
            res.merge(res_net.get());
        }
        return OK(res);
    }

    Result<std::map<Gate*, std::set<Gate*>>> NetlistTraversalDecorator::get_next_sequential_gates_map(bool successors, const std::set<PinType>& forbidden_pins) const
    {
        std::map<Gate*, std::set<Gate*>> seq_gate_map;
        std::unordered_map<const Net*, std::set<Gate*>> cache = {};

        for (auto* sg : m_netlist.get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::sequential); }))
        {
            if (const auto res = this->get_next_sequential_gates(sg, successors, forbidden_pins, &cache); res.is_ok())
            {
                seq_gate_map[sg] = res.get();
            }
            else
            {
                return ERR(res.get_error());
            }
        }

        return OK(std::move(seq_gate_map));
    }

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_combinational_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache) const
    {
        const auto entry = [&forbidden_pins](const Endpoint* ep) { return forbidden_pins.find(ep->get_pin()->get_type()) == forbidden_pins.end(); };
        const auto match = [](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); };

        std::unordered_map<const Net*, std::set<Gate*>> local_store;
        return get_gates_memoized(net, successors, match, TraversalStop::at_mismatch, entry, entry, (cache != nullptr) ? *cache : local_store);
    }

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_combinational_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("gate does not belong to netlist");
        }

        std::set<Gate*> res;
        std::unordered_map<const Net*, std::set<Gate*>> local_store;
        for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (forbidden_pins.find(exit_ep->get_pin()->get_type()) != forbidden_pins.end())
            {
                continue;
            }

            auto res_net = get_next_combinational_gates(exit_ep->get_net(), successors, forbidden_pins, (cache != nullptr) ? cache : &local_store);
            if (res_net.is_error())
            {
                return ERR_APPEND(res_net.get_error(), "cannot get next combinational gates of gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
            }
            res.merge(res_net.get());
        }
        return OK(res);
    }

    Result<std::optional<u32>> NetlistTraversalDecorator::get_shortest_path_distance(const Gate* start_gate,
                                                                                     const Gate* end_gate,
                                                                                     const PinDirection& direction,
                                                                                     const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                     const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (direction == PinDirection::output || direction == PinDirection::input)
        {
            std::unordered_set<Gate*> visited;
            u32 distance = 0;

            Gate* _start_gate          = start_gate->get_netlist()->get_gate_by_id(start_gate->get_id());
            std::vector<Gate*> current = {_start_gate};
            std::vector<Gate*> next;

            while (true)
            {
                distance++;

                for (const auto& curr_g : current)
                {
                    for (const auto& exit_ep : (direction == PinDirection::output) ? curr_g->get_fan_out_endpoints() : curr_g->get_fan_in_endpoints())
                    {
                        if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, distance))
                        {
                            continue;
                        }

                        for (const auto& entry_ep : (direction == PinDirection::output) ? exit_ep->get_net()->get_destinations() : exit_ep->get_net()->get_sources())
                        {
                            if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, distance))
                            {
                                continue;
                            }

                            const auto next_g = entry_ep->get_gate();

                            if (const auto it = visited.find(next_g); it != visited.end())
                            {
                                continue;
                            }
                            visited.insert(next_g);

                            if (next_g == end_gate)
                            {
                                return OK(distance);
                            }

                            next.push_back(next_g);
                        }
                    }
                }

                if (next.empty())
                {
                    break;
                }

                current = next;
                next.clear();
            }

            return OK({});
        }

        if (direction == PinDirection::inout)
        {
            const auto res_backward = get_shortest_path_distance(start_gate, end_gate, PinDirection::input);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_shortest_path_distance(start_gate, end_gate, PinDirection::output);
            if (res_forward.is_error())
            {
                return res_forward;
            }

            const auto distance_backward = res_backward.get();
            const auto distance_forward  = res_forward.get();

            if (!distance_forward.has_value() && !distance_backward.has_value())
            {
                return OK({});
            }

            if (!distance_backward.has_value())
            {
                return OK(distance_forward);
            }

            if (!distance_forward.has_value())
            {
                return OK(distance_backward);
            }

            if (distance_backward.value() < distance_forward.value())
            {
                return OK(distance_backward);
            }

            return OK(distance_forward);
        }

        return ERR("cannot get shortest path distance between Gate " + start_gate->get_name() + " with ID " + std::to_string(start_gate->get_id()) + " and Gate " + end_gate->get_name() + " with ID "
                   + std::to_string(end_gate->get_id()) + ": pin direction " + enum_to_string(direction) + " is not supported");
    }

    Result<std::optional<std::vector<Gate*>>> NetlistTraversalDecorator::get_shortest_path_to(const Gate* start_gate,
                                                                                              const std::function<bool(const Gate*)>& is_target,
                                                                                              const PinDirection& direction,
                                                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                              const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        const auto reconstruct_shortest_path = [](const Gate* start_gate, const Gate* end_gate, const std::unordered_map<Gate*, Gate*>& origin_map) -> Result<std::optional<std::vector<Gate*>>> {
            Gate* _start_gate       = start_gate->get_netlist()->get_gate_by_id(start_gate->get_id());
            Gate* _end_gate         = end_gate->get_netlist()->get_gate_by_id(end_gate->get_id());
            std::vector<Gate*> path = {_end_gate};

            Gate* curr = _end_gate;
            do
            {
                if (const auto& it = origin_map.find(curr); it != origin_map.end())
                {
                    path.push_back(it->second);
                    curr = it->second;
                }
                else
                {
                    return ERR("cannot reconstruct shortest path between Gate " + start_gate->get_name() + " with ID " + std::to_string(start_gate->get_id()) + " and Gate " + end_gate->get_name()
                               + " with ID " + std::to_string(end_gate->get_id()) + ": failed to find origin for Gate " + curr->get_name() + " with ID " + std::to_string(curr->get_id()));
                }
            } while (curr != start_gate);

            std::reverse(path.begin(), path.end());

            return OK(path);
        };

        if (direction == PinDirection::output || direction == PinDirection::input)
        {
            std::unordered_map<Gate*, Gate*> origin_map;

            u32 distance = 0;

            Gate* _start_gate          = start_gate->get_netlist()->get_gate_by_id(start_gate->get_id());
            std::vector<Gate*> current = {_start_gate};
            std::vector<Gate*> next;

            while (true)
            {
                distance++;

                for (const auto& curr_g : current)
                {
                    for (const auto& exit_ep : (direction == PinDirection::output) ? curr_g->get_fan_out_endpoints() : curr_g->get_fan_in_endpoints())
                    {
                        if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, distance))
                        {
                            continue;
                        }

                        for (const auto& entry_ep : (direction == PinDirection::output) ? exit_ep->get_net()->get_destinations() : exit_ep->get_net()->get_sources())
                        {
                            if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, distance))
                            {
                                continue;
                            }

                            const auto next_g = entry_ep->get_gate();

                            if (const auto it = origin_map.find(next_g); it != origin_map.end())
                            {
                                continue;
                            }
                            origin_map.insert({next_g, curr_g});

                            if (is_target(next_g))
                            {
                                return reconstruct_shortest_path(start_gate, next_g, origin_map);
                            }

                            next.push_back(next_g);
                        }
                    }
                }

                if (next.empty())
                {
                    break;
                }

                current = next;
                next.clear();
            }

            return OK({});
        }

        if (direction == PinDirection::inout)
        {
            const auto res_backward = get_shortest_path_to(start_gate, is_target, PinDirection::input, exit_endpoint_filter, entry_endpoint_filter);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_shortest_path_to(start_gate, is_target, PinDirection::output, exit_endpoint_filter, entry_endpoint_filter);
            if (res_forward.is_error())
            {
                return res_forward;
            }

            const auto path_backward = res_backward.get();
            const auto path_forward  = res_forward.get();

            if (!path_forward.has_value() && !path_backward.has_value())
            {
                return OK({});
            }

            if (!path_backward.has_value())
            {
                return OK(path_forward);
            }

            if (!path_forward.has_value())
            {
                return OK(path_backward);
            }

            if (path_backward.value().size() < path_forward.value().size())
            {
                return OK(path_backward);
            }

            return OK(path_forward);
        }

        return ERR("cannot get shortest path from Gate " + start_gate->get_name() + " with ID " + std::to_string(start_gate->get_id()) + ": pin direction " + enum_to_string(direction)
                   + " is not supported");
    }

    Result<std::optional<std::vector<Gate*>>> NetlistTraversalDecorator::get_shortest_path(const Gate* start_gate,
                                                                                           const Gate* end_gate,
                                                                                           const PinDirection& direction,
                                                                                           const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                           const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (end_gate == nullptr)
        {
            return ERR("nullptr given as end gate");
        }

        return get_shortest_path_to(
            start_gate, [end_gate](const Gate* gate) { return gate == end_gate; }, direction, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::optional<std::vector<Gate*>>> NetlistTraversalDecorator::get_shortest_path(const Gate* start_gate,
                                                                                           const Module* end_module,
                                                                                           const PinDirection& direction,
                                                                                           const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                           const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (end_module == nullptr)
        {
            return ERR("nullptr given as end module");
        }

        const auto gates = end_module->get_gates(nullptr, true);
        const std::unordered_set<const Gate*> end_gates(gates.begin(), gates.end());

        // A start gate that is already inside the module has arrived: the path is the gate itself. Searching from
        // here instead would walk to a neighbour and report a path of two, and reporting nothing would not be
        // distinguishable from the module being unreachable.
        if (end_gates.find(start_gate) != end_gates.end())
        {
            Gate* gate = start_gate->get_netlist()->get_gate_by_id(start_gate->get_id());
            return OK(std::optional<std::vector<Gate*>>({gate}));
        }

        return get_shortest_path_to(
            start_gate, [&end_gates](const Gate* gate) { return end_gates.find(gate) != end_gates.end(); }, direction, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::vector<std::vector<Gate*>>> NetlistTraversalDecorator::get_shortest_path(const Module* start_module,
                                                                                         const Module* end_module,
                                                                                         const PinDirection& direction,
                                                                                         const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                         const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (start_module == nullptr)
        {
            return ERR("nullptr given as start module");
        }
        if (end_module == nullptr)
        {
            return ERR("nullptr given as end module");
        }

        // One search per gate of the start module, keeping every path that ties for the shortest. Searching from each
        // gate rather than towards each one keeps the paths running from start to end, so that a caller does not have
        // to know which end a path was grown from.
        std::vector<std::vector<Gate*>> shortest;
        for (Gate* start_gate : start_module->get_gates(nullptr, true))
        {
            const auto res = get_shortest_path(start_gate, end_module, direction, exit_endpoint_filter, entry_endpoint_filter);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(),
                                  "cannot get shortest path between Module " + start_module->get_name() + " with ID " + std::to_string(start_module->get_id()) + " and Module "
                                      + end_module->get_name() + " with ID " + std::to_string(end_module->get_id()) + ": failed to search from Gate " + start_gate->get_name());
            }

            const auto& path = res.get();
            if (!path.has_value() || path.value().empty())
            {
                continue;
            }

            if (shortest.empty() || path.value().size() < shortest.front().size())
            {
                shortest = {path.value()};
            }
            else if (path.value().size() == shortest.front().size())
            {
                shortest.push_back(path.value());
            }
        }

        return OK(shortest);
    }



        Result<std::vector<Gate*>>
        NetlistTraversalDecorator::get_gate_chain(Gate* start_gate, const std::vector<const GatePin*>& input_pins, const std::vector<const GatePin*>& output_pins, const std::function<bool(const Gate*)>& filter) const
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

        Result<std::vector<Gate*>> NetlistTraversalDecorator::get_complex_gate_chain(Gate* start_gate,
                                                          const std::vector<GateType*>& chain_types,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& input_pins,
                                                          const std::map<GateType*, std::vector<const GatePin*>>& output_pins,
                                                                                            const std::function<bool(const Gate*)>& filter) const
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

}    // namespace hal