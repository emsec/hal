

#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"

#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetlistAbstraction::NetlistAbstraction(const Netlist* netlist,
                                           const std::vector<Gate*>& gates,
                                           const bool include_all_netlist_gates,
                                           const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter,
                                           const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter)
    {
        const auto nl_trav_dec = NetlistTraversalDecorator(*netlist);

        // transform gates into set to check fast if a gate is part of abstraction
        const auto gates_set = utils::to_unordered_set(gates);

        for (const Gate* gate : include_all_netlist_gates ? netlist->get_gates() : gates)
        {
            // TODO remove debug print
            // std::cout << gate->get_id() << std::endl;

            // gather all successors
            for (Endpoint* ep_out : gate->get_fan_out_endpoints())
            {
                m_successors.insert({ep_out, {}});
                const auto successors = nl_trav_dec.get_next_matching_endpoints(
                    ep_out,
                    true,
                    [gates_set](const auto& ep) { return ep->is_destination_pin() && gates_set.find(ep->get_gate()) != gates_set.end(); },
                    false,
                    exit_endpoint_filter,
                    entry_endpoint_filter);

                if (successors.is_error())
                {
                    log_error("NetlistAbstractionDecorator", "cannot create netlist abstraction: {}", successors.get_error().get());
                }

                for (Endpoint* ep : successors.get())
                {
                    m_successors.at(ep_out).push_back(ep);
                }
            }

            // gather all global output succesors
            for (Endpoint* ep_out : gate->get_fan_out_endpoints())
            {
                m_global_output_successors.insert({ep_out, {}});

                const auto destinations = nl_trav_dec.get_next_matching_endpoints(
                    ep_out, true, [](const auto& ep) { return ep->is_source_pin() && ep->get_net()->is_global_output_net(); }, false, exit_endpoint_filter, entry_endpoint_filter);

                if (destinations.is_error())
                {
                    log_error("NetlistAbstractionDecorator", "cannot create netlist abstraction: {}", destinations.get_error().get());
                }

                for (const auto* ep : destinations.get())
                {
                    m_global_output_successors.at(ep_out).push_back({ep->get_net()});
                }
            }

            // gather all predecessors
            for (Endpoint* ep_in : gate->get_fan_in_endpoints())
            {
                m_predecessors.insert({ep_in, {}});

                const auto predecessors =
                    nl_trav_dec.get_next_matching_endpoints(ep_in, false, [gates_set](const auto& ep) { return ep->is_source_pin() && gates_set.find(ep->get_gate()) != gates_set.end(); });

                if (predecessors.is_error())
                {
                    log_error("NetlistAbstractionDecorator", "cannot create netlist abstraction: {}", predecessors.get_error().get());
                }

                for (Endpoint* ep : predecessors.get())
                {
                    m_predecessors.at(ep_in).push_back(ep);
                }
            }

            // gather all global input predecessors
            for (Endpoint* ep_in : gate->get_fan_in_endpoints())
            {
                m_global_input_predecessors.insert({ep_in, {}});

                const auto predecessors = nl_trav_dec.get_next_matching_endpoints(ep_in, false, [](const auto& ep) { return ep->is_destination_pin() && ep->get_net()->is_global_input_net(); });

                if (predecessors.is_error())
                {
                    log_error("NetlistAbstractionDecorator", "cannot create netlist abstraction: {}", predecessors.get_error().get());
                }

                for (const auto* ep : predecessors.get())
                {
                    m_global_input_predecessors.at(ep_in).push_back({ep->get_net()});
                }
            }
        }

        return;
    }

    std::vector<Endpoint*> NetlistAbstraction::get_predecessors(const Gate* gate) const
    {
        std::vector<Endpoint*> predecessors;
        for (auto* ep : gate->get_fan_out_endpoints())
        {
            const auto new_predecessors = get_predecessors(ep);
            predecessors.insert(predecessors.end(), new_predecessors.begin(), new_predecessors.end());
        }

        return predecessors;
    }

    std::vector<Endpoint*> NetlistAbstraction::get_predecessors(const Endpoint* endpoint) const
    {
        const auto it = m_predecessors.find(endpoint);
        if (it == m_predecessors.end())
        {
            log_error("netlist_abstraction_decorator", "Endpoint does not have any predecessor edges in the abstraction!");
            return {};
        }
        return it->second;
    }

    std::vector<Gate*> NetlistAbstraction::get_unique_predecessors(const Gate* gate) const
    {
        std::vector<Gate*> predecessors;
        for (auto* ep : gate->get_fan_out_endpoints())
        {
            for (const auto* pred_ep : get_predecessors(ep))
            {
                predecessors.push_back(pred_ep->get_gate());
            }
        }

        std::sort(predecessors.begin(), predecessors.end());
        predecessors.erase(std::unique(predecessors.begin(), predecessors.end()), predecessors.end());

        return predecessors;
    }

    std::vector<Gate*> NetlistAbstraction::get_unique_predecessors(const Endpoint* endpoint) const
    {
        std::vector<Gate*> predecessors;
        for (const auto* pred_ep : get_predecessors(endpoint))
        {
            predecessors.push_back(pred_ep->get_gate());
        }

        std::sort(predecessors.begin(), predecessors.end());
        predecessors.erase(std::unique(predecessors.begin(), predecessors.end()), predecessors.end());

        return predecessors;
    }

    std::vector<Endpoint*> NetlistAbstraction::get_successors(const Gate* gate) const
    {
        std::vector<Endpoint*> successors;
        for (auto* ep : gate->get_fan_in_endpoints())
        {
            const auto new_successors = get_successors(ep);
            successors.insert(successors.end(), new_successors.begin(), new_successors.end());
        }

        return successors;
    }

    std::vector<Endpoint*> NetlistAbstraction::get_successors(const Endpoint* endpoint) const
    {
        const auto it = m_successors.find(endpoint);
        if (it == m_successors.end())
        {
            log_error("netlist_abstraction_decorator", "Endpoint does not have any successor edges in the abstraction!");
            return {};
        }
        return it->second;
    }

    std::vector<Gate*> NetlistAbstraction::get_unique_successors(const Gate* gate) const
    {
        std::vector<Gate*> successors;
        for (auto* ep : gate->get_fan_in_endpoints())
        {
            for (const auto* succ_ep : get_successors(ep))
            {
                successors.push_back(succ_ep->get_gate());
            }
        }

        std::sort(successors.begin(), successors.end());
        successors.erase(std::unique(successors.begin(), successors.end()), successors.end());

        return successors;
    }

    std::vector<Gate*> NetlistAbstraction::get_unique_successors(const Endpoint* endpoint) const
    {
        std::vector<Gate*> successors;
        for (const auto* succ_ep : get_successors(endpoint))
        {
            successors.push_back(succ_ep->get_gate());
        }

        std::sort(successors.begin(), successors.end());
        successors.erase(std::unique(successors.begin(), successors.end()), successors.end());

        return successors;
    }

    std::vector<Net*> NetlistAbstraction::get_global_input_predecessors(const Endpoint* endpoint) const
    {
        const auto it = m_global_input_predecessors.find(endpoint);
        if (it == m_global_input_predecessors.end())
        {
            log_error("netlist_abstraction_decorator", "Endpoint does not have any global input predecessor edges in the abstraction!");
            return {};
        }
        return it->second;
    }

    std::vector<Net*> NetlistAbstraction::get_global_output_successors(const Endpoint* endpoint) const
    {
        const auto it = m_global_output_successors.find(endpoint);
        if (it == m_global_output_successors.end())
        {
            log_error("netlist_abstraction_decorator", "Endpoint does not have any global output successor edges in the abstraction!");
            return {};
        }
        return it->second;
    }

    NetlistAbstractionDecorator::NetlistAbstractionDecorator(const hal::NetlistAbstraction& abstraction) : m_abstraction(abstraction) {};

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance_internal(const std::vector<Endpoint*>& start,
                                                                                                const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                                                const PinDirection& direction,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (direction == PinDirection::output || direction == PinDirection::input)
        {
            u32 distance = 0;

            std::unordered_set<Endpoint*> visited;

            std::vector<Endpoint*> current;
            std::vector<Endpoint*> next;

            // check whether start already fullfills target or exit filter
            for (auto* start_ep : start)
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(start_ep, distance))
                {
                    continue;
                }

                if (target_filter(start_ep, m_abstraction))
                {
                    return OK(distance);
                }

                current.push_back(start_ep);
            }

            while (true)
            {
                distance++;

                for (const auto& exit_ep : current)
                {
                    for (const auto& entry_ep : (direction == PinDirection::output) ? m_abstraction.get_successors(exit_ep) : m_abstraction.get_predecessors(exit_ep))
                    {
                        if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, distance))
                        {
                            continue;
                        }

                        if (target_filter(entry_ep, m_abstraction))
                        {
                            return OK(distance);
                        }

                        const auto next_g = entry_ep->get_gate();

                        for (const auto& next_ep : (direction == PinDirection::output) ? next_g->get_fan_out_endpoints() : next_g->get_fan_in_endpoints())
                        {
                            if (const auto it = visited.find(next_ep); it != visited.end())
                            {
                                continue;
                            }
                            visited.insert(next_ep);

                            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(next_ep, distance))
                            {
                                continue;
                            }

                            if (target_filter(next_ep, m_abstraction))
                            {
                                return OK(distance);
                            }

                            next.push_back(next_ep);
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
            const auto res_backward = get_shortest_path_distance_internal(start, target_filter, PinDirection::input, exit_endpoint_filter, entry_endpoint_filter);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_shortest_path_distance_internal(start, target_filter, PinDirection::output, exit_endpoint_filter, entry_endpoint_filter);
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

        return ERR("cannot get shortest path distance between endpoints and given target filter condition: pin direction " + enum_to_string(direction) + " is not supported");
    }

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance(Endpoint* start,
                                                                                       const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                                       const PinDirection& direction,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (start->get_pin()->get_direction() != direction)
        {
            return get_shortest_path_distance(start->get_gate(), target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
        }

        return get_shortest_path_distance_internal({start}, target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance(const Gate* start,
                                                                                       const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                                       const PinDirection& direction,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        const auto start_endpoints = (direction == PinDirection::output) ? start->get_fan_out_endpoints() : start->get_fan_in_endpoints();

        return get_shortest_path_distance_internal(start_endpoints, target_filter, direction, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance(const Gate* start,
                                                                                       const Gate* target,
                                                                                       const PinDirection& direction,
                                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter) const
    {
        return get_shortest_path_distance(
            start,
            [target](const auto* ep, const auto& _abstraction) {
                UNUSED(_abstraction);
                return ep->get_gate() == target;
            },
            direction,
            exit_endpoint_filter,
            entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_internal(const std::vector<Endpoint*>& start,
                                                                                          const PinDirection& direction,
                                                                                          const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                          bool continue_on_match,
                                                                                          const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                          const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        std::set<Gate*> res;
        u32 distance = 0;

        std::unordered_set<Endpoint*> visited;

        std::vector<Endpoint*> current;
        std::vector<Endpoint*> next;

        // check whether start already fullfills target or exit filter
        for (auto* start_ep : start)
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(start_ep, distance))
            {
                continue;
            }

            current.push_back(start_ep);
        }

        while (true)
        {
            distance++;

            for (const auto& exit_ep : current)
            {
                for (const auto& entry_ep : (direction == PinDirection::output) ? m_abstraction.get_successors(exit_ep) : m_abstraction.get_predecessors(exit_ep))
                {
                    if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, distance))
                    {
                        continue;
                    }

                    const auto next_g = entry_ep->get_gate();

                    if (target_gate_filter(next_g))
                    {
                        res.insert(next_g);

                        if (!continue_on_match)
                        {
                            continue;
                        }
                    }

                    for (const auto& next_ep : (direction == PinDirection::output) ? next_g->get_fan_out_endpoints() : next_g->get_fan_in_endpoints())
                    {
                        if (const auto it = visited.find(next_ep); it != visited.end())
                        {
                            continue;
                        }
                        visited.insert(next_ep);

                        if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(next_ep, distance))
                        {
                            continue;
                        }

                        next.push_back(next_ep);
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

        return OK(res);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates(Endpoint* endpoint,
                                                                                 const PinDirection& direction,
                                                                                 const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                 bool continue_on_match,
                                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (endpoint == nullptr)
        {
            return ERR("nullptr given as endpoint");
        }

        if (!target_gate_filter)
        {
            return ERR("no target gate filter specified");
        }

        if (endpoint->get_pin()->get_direction() != direction)
        {
            return get_next_matching_gates(endpoint->get_gate(), direction, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
        }

        return get_next_matching_gates_internal({endpoint}, direction, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates(const Gate* gate,
                                                                                 const PinDirection& direction,
                                                                                 const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                 bool continue_on_match,
                                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                 const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!target_gate_filter)
        {
            return ERR("no target gate filter specified");
        }

        std::vector<Endpoint*> start;
        for (auto* exit_ep : (direction == PinDirection::output) ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
            {
                continue;
            }

            start.push_back(exit_ep);
        }

        return get_next_matching_gates_internal(start, direction, target_gate_filter, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_until_internal(const std::vector<Endpoint*>& start,
                                                                                                const PinDirection& direction,
                                                                                                const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                                bool continue_on_mismatch,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        std::set<Gate*> res;
        u32 distance = 0;

        std::unordered_set<Endpoint*> visited;

        std::vector<Endpoint*> current;
        std::vector<Endpoint*> next;

        // check whether start already fullfills target or exit filter
        for (auto* start_ep : start)
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(start_ep, distance))
            {
                continue;
            }

            current.push_back(start_ep);
        }

        while (true)
        {
            distance++;

            for (const auto& exit_ep : current)
            {
                for (const auto& entry_ep : (direction == PinDirection::output) ? m_abstraction.get_successors(exit_ep) : m_abstraction.get_predecessors(exit_ep))
                {
                    if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, distance))
                    {
                        continue;
                    }

                    const auto next_g = entry_ep->get_gate();

                    if (!target_gate_filter(next_g))
                    {
                        res.insert(next_g);

                        if (!continue_on_mismatch)
                        {
                            continue;
                        }
                    }

                    for (const auto& next_ep : (direction == PinDirection::output) ? next_g->get_fan_out_endpoints() : next_g->get_fan_in_endpoints())
                    {
                        if (const auto it = visited.find(next_ep); it != visited.end())
                        {
                            continue;
                        }
                        visited.insert(next_ep);

                        if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(next_ep, distance))
                        {
                            continue;
                        }

                        next.push_back(next_ep);
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

        return OK(res);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_until(Endpoint* endpoint,
                                                                                       const PinDirection& direction,
                                                                                       const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                       bool continue_on_mismatch,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (endpoint == nullptr)
        {
            return ERR("nullptr given as endpoint");
        }

        if (!target_gate_filter)
        {
            return ERR("no target gate filter specified");
        }

        if (endpoint->get_pin()->get_direction() != direction)
        {
            return get_next_matching_gates_until(endpoint->get_gate(), direction, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
        }

        return get_next_matching_gates_until_internal({endpoint}, direction, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_until(const Gate* gate,
                                                                                       const PinDirection& direction,
                                                                                       const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                       bool continue_on_mismatch,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!target_gate_filter)
        {
            return ERR("no target gate filter specified");
        }

        std::vector<Endpoint*> start;
        for (auto* exit_ep : (direction == PinDirection::output) ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
            {
                continue;
            }

            start.push_back(exit_ep);
        }

        return get_next_matching_gates_internal(start, direction, target_gate_filter, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
    }

}    // namespace hal