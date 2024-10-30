#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetlistTraversalDecorator::NetlistTraversalDecorator(const Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_matching_gates(const Net* net,
                                                                               bool successors,
                                                                               const std::function<bool(const Gate*)>& target_gate_filter,
                                                                               bool continue_on_match,
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

        if (!target_gate_filter)
        {
            return ERR("no target gate filter specified");
        }

        std::unordered_set<const Net*> visited;
        std::vector<const Net*> stack = {net};
        std::vector<const Net*> previous;
        std::set<Gate*> res;
        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (const auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, previous.size() + 1))
                {
                    continue;
                }

                auto* gate = entry_ep->get_gate();

                if (target_gate_filter(gate))
                {
                    res.insert(gate);

                    if (!continue_on_match)
                    {
                        continue;
                    }
                }

                for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
                {
                    const Net* exit_net = exit_ep->get_net();

                    if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, previous.size() + 1))
                    {
                        continue;
                    }

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
        if (net == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(net))
        {
            return ERR("net does not belong to netlist");
        }

        if (!target_gate_filter)
        {
            return ERR("no target gate filter specified");
        }

        std::unordered_set<const Net*> visited;
        std::vector<const Net*> stack = {net};
        std::vector<const Net*> previous;
        std::set<Gate*> res;
        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (const auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, previous.size() + 1))
                {
                    continue;
                }

                auto* g = entry_ep->get_gate();

                if (target_gate_filter(g))
                {
                    res.insert(g);
                }
                else
                {
                    if (!continue_on_mismatch)
                    {
                        continue;
                    }
                }

                for (const auto* exit_ep : successors ? g->get_fan_out_endpoints() : g->get_fan_in_endpoints())
                {
                    if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, previous.size() + 1))
                    {
                        continue;
                    }

                    const Net* n = exit_ep->get_net();
                    if (visited.find(n) == visited.end())
                    {
                        stack.push_back(n);
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
        if (net == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(net))
        {
            return ERR("net does not belong to netlist");
        }

        std::unordered_set<const Net*> visited;
        std::vector<const Net*> stack = {net};
        std::vector<const Net*> previous;
        std::set<Gate*> res;
        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            u32 current_depth = previous.size() + 1;
            visited.insert(current);

            bool added = false;
            for (const auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                if (max_depth != 0 && current_depth > max_depth)
                {
                    continue;
                }

                auto* g = entry_ep->get_gate();

                if ((target_gate_filter == nullptr) || target_gate_filter(g))
                {
                    res.insert(g);
                }

                for (const auto* exit_ep : successors ? g->get_fan_out_endpoints() : g->get_fan_in_endpoints())
                {
                    if (max_depth != 0 && current_depth == max_depth)
                    {
                        continue;
                    }

                    const Net* n = exit_ep->get_net();
                    if (visited.find(n) == visited.end())
                    {
                        stack.push_back(n);
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

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_sequential_gates(const Net* net, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache) const
    {
        if (net == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(net))
        {
            return ERR("net does not belong to netlist");
        }

        std::unordered_set<const Net*> visited;
        std::vector<const Net*> stack = {net};
        std::vector<const Net*> previous;
        std::set<Gate*> res;
        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (const auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                auto entry_pin = entry_ep->get_pin();
                auto* gate     = entry_ep->get_gate();

                // stop traversal if gate is sequential
                if (gate->get_type()->has_property(GateTypeProperty::sequential))
                {
                    // stop traversal on forbidden pins
                    if (forbidden_pins.find(entry_pin->get_type()) != forbidden_pins.end())
                    {
                        continue;
                    }

                    // only add gate to result if it has not been reached through a forbidden pin (e.g., control pin)
                    res.insert(gate);

                    // update cache
                    if (cache)
                    {
                        (*cache)[current].insert(gate);
                        for (const auto* n : previous)
                        {
                            (*cache)[n].insert(gate);
                        }
                    }
                }
                else
                {
                    for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
                    {
                        const Net* exit_net     = exit_ep->get_net();
                        const GatePin* exit_pin = exit_ep->get_pin();

                        // stop traversal on forbidden pins
                        if (forbidden_pins.find(exit_pin->get_type()) != forbidden_pins.end())
                        {
                            continue;
                        }

                        if (cache)
                        {
                            if (const auto it = cache->find(exit_net); it != cache->end())
                            {
                                const auto& cached_gates = std::get<1>(*it);

                                // append cached gates to result
                                res.insert(cached_gates.begin(), cached_gates.end());

                                // update cache
                                (*cache)[current].insert(cached_gates.begin(), cached_gates.end());
                                for (const auto* n : previous)
                                {
                                    (*cache)[n].insert(cached_gates.begin(), cached_gates.end());
                                }

                                continue;
                            }
                        }

                        if (visited.find(exit_net) == visited.end())
                        {
                            stack.push_back(exit_net);
                            added = true;
                        }
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

    Result<std::set<Gate*>>
        NetlistTraversalDecorator::get_next_sequential_gates(const Gate* gate, bool successors, const std::set<PinType>& forbidden_pins, std::unordered_map<const Net*, std::set<Gate*>>* cache) const
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
            const auto* exit_net = exit_ep->get_net();
            const auto* exit_pin = exit_ep->get_pin();

            // stop traversal on forbidden pins
            if (forbidden_pins.find(exit_pin->get_type()) != forbidden_pins.end())
            {
                continue;
            }

            if (cache)
            {
                if (const auto it = cache->find(exit_net); it != cache->end())
                {
                    const auto& cached_gates = std::get<1>(*it);

                    // append cached gates to result
                    res.insert(cached_gates.begin(), cached_gates.end());

                    continue;
                }
            }

            const auto next_res = this->get_next_sequential_gates(exit_ep->get_net(), successors, forbidden_pins, cache);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
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
        if (net == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(net))
        {
            return ERR("net does not belong to netlist");
        }

        std::unordered_set<const Net*> visited;
        std::vector<const Net*> stack = {net};
        std::vector<const Net*> previous;
        std::set<Gate*> res;
        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (const auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                auto* gate            = entry_ep->get_gate();
                const auto* entry_pin = entry_ep->get_pin();
                if (!gate->get_type()->has_property(GateTypeProperty::combinational))
                {
                    // stop traversal if not combinational
                    continue;
                }

                // stop traversal on forbidden pins
                if (forbidden_pins.find(entry_pin->get_type()) != forbidden_pins.end())
                {
                    continue;
                }

                // add to result if gate is combinational
                res.insert(gate);

                // update cache
                if (cache)
                {
                    (*cache)[current].insert(gate);
                    for (const auto* n : previous)
                    {
                        (*cache)[n].insert(gate);
                    }
                }

                for (const auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
                {
                    const Net* exit_net     = exit_ep->get_net();
                    const GatePin* exit_pin = exit_ep->get_pin();

                    // stop traversal on forbidden pins
                    if (forbidden_pins.find(exit_pin->get_type()) != forbidden_pins.end())
                    {
                        continue;
                    }

                    if (cache)
                    {
                        if (const auto it = cache->find(exit_net); it != cache->end())
                        {
                            const auto& cached_gates = std::get<1>(*it);

                            // append cached gates to result
                            res.insert(cached_gates.begin(), cached_gates.end());

                            continue;
                        }
                    }

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

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_combinational_gates(const Gate* gate,
                                                                                    bool successors,
                                                                                    const std::set<PinType>& forbidden_pins,
                                                                                    std::unordered_map<const Net*, std::set<Gate*>>* cache) const
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
            const auto* exit_net = exit_ep->get_net();
            const auto* exit_pin = exit_ep->get_pin();

            // stop traversal on forbidden pins
            if (forbidden_pins.find(exit_pin->get_type()) != forbidden_pins.end())
            {
                continue;
            }

            if (cache)
            {
                if (const auto it = cache->find(exit_net); it != cache->end())
                {
                    const auto& cached_gates = std::get<1>(*it);

                    // append cached gates to result
                    res.insert(cached_gates.begin(), cached_gates.end());

                    continue;
                }
            }

            const auto next_res = this->get_next_combinational_gates(exit_ep->get_net(), successors, forbidden_pins, cache);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
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

    Result<std::optional<std::vector<Gate*>>> NetlistTraversalDecorator::get_shortest_path(const Gate* start_gate,
                                                                                           const Gate* end_gate,
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

                            if (next_g == end_gate)
                            {
                                return reconstruct_shortest_path(start_gate, end_gate, origin_map);
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
            const auto res_backward = get_shortest_path(start_gate, end_gate, PinDirection::input);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_shortest_path(start_gate, end_gate, PinDirection::output);
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

        return ERR("cannot get shortest path between Gate " + start_gate->get_name() + " with ID " + std::to_string(start_gate->get_id()) + " and Gate " + end_gate->get_name() + " with ID "
                   + std::to_string(end_gate->get_id()) + ": pin direction " + enum_to_string(direction) + " is not supported");
    }

    Result<std::set<Endpoint*>> NetlistTraversalDecorator::get_next_matching_endpoints(const Net* net,
                                                                                       bool successors,
                                                                                       const std::function<bool(const Endpoint*)>& target_endpoint_filter,
                                                                                       bool continue_on_match,
                                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter) const
    {
        if (net == nullptr)
        {
            return ERR("nullptr given as net");
        }

        if (!m_netlist.is_net_in_netlist(net))
        {
            return ERR("net does not belong to netlist");
        }

        if (!target_endpoint_filter)
        {
            return ERR("no target endpoint filter specified");
        }

        std::unordered_set<const Net*> visited;
        std::vector<const Net*> stack = {net};
        std::vector<const Net*> previous;
        std::set<Endpoint*> res;
        while (!stack.empty())
        {
            const Net* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (auto* entry_ep : successors ? current->get_destinations() : current->get_sources())
            {
                if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, previous.size() + 1))
                {
                    continue;
                }

                if (target_endpoint_filter(entry_ep))
                {
                    res.insert(entry_ep);

                    if (!continue_on_match)
                    {
                        continue;
                    }
                }

                auto* gate = entry_ep->get_gate();

                for (auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
                {
                    if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, previous.size() + 1))
                    {
                        continue;
                    }

                    if (target_endpoint_filter(exit_ep))
                    {
                        res.insert(exit_ep);

                        if (!continue_on_match)
                        {
                            continue;
                        }
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

    Result<std::set<Endpoint*>> NetlistTraversalDecorator::get_next_matching_endpoints(const Gate* gate,
                                                                                       bool successors,
                                                                                       const std::function<bool(const Endpoint*)>& target_endpoint_filter,
                                                                                       bool continue_on_match,
                                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("net does not belong to netlist");
        }

        if (!target_endpoint_filter)
        {
            return ERR("no target endpoint filter specified");
        }

        std::set<Endpoint*> res;
        for (auto* exit_ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
            {
                continue;
            }

            if (target_endpoint_filter(exit_ep))
            {
                res.insert(exit_ep);

                if (!continue_on_match)
                {
                    continue;
                }
            }

            const auto* exit_net = exit_ep->get_net();
            const auto next_res  = this->get_next_matching_endpoints(exit_net, successors, target_endpoint_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

}    // namespace hal