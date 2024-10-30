

#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"

#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{

    NetlistAbstraction::NetlistAbstraction(const std::vector<Gate*>& gates,
                                           const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter,
                                           const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter)
    {
        if (gates.empty())
        {
            return;
        }

        const Netlist* nl      = gates.front()->get_netlist();
        const auto nl_trav_dec = NetlistTraversalDecorator(*nl);

        // transform gates into set to check fast if a gate is part of abstraction
        const auto gates_set = utils::to_set(gates);

        for (const Gate* gate : gates)
        {
            // gather all successors
            for (Endpoint* ep_out : gate->get_fan_out_endpoints())
            {
                m_successors.insert({ep_out, {}});

                const auto* net       = ep_out->get_net();
                const auto* pin       = ep_out->get_pin();
                const auto successors = nl_trav_dec.get_next_matching_endpoints(
                    net,
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

                const auto* net         = ep_out->get_net();
                const auto* pin         = ep_out->get_pin();
                const auto destinations = nl_trav_dec.get_next_matching_endpoints(
                    net, true, [](const auto& ep) { return ep->is_source_pin() && ep->get_net()->is_global_output_net(); }, false, exit_endpoint_filter, entry_endpoint_filter);

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

                const auto* net = ep_in->get_net();
                const auto* pin = ep_in->get_pin();
                const auto predecessors =
                    nl_trav_dec.get_next_matching_endpoints(net, false, [gates_set](const auto& ep) { return ep->is_source_pin() && gates_set.find(ep->get_gate()) != gates_set.end(); });

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

                const auto* net         = ep_in->get_net();
                const auto* pin         = ep_in->get_pin();
                const auto predecessors = nl_trav_dec.get_next_matching_endpoints(net, false, [](const auto& ep) { return ep->is_destination_pin() && ep->get_net()->is_global_input_net(); });

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
            const auto new_predecessors = m_predecessors.at(ep);
            predecessors.insert(predecessors.end(), new_predecessors.begin(), new_predecessors.end());
        }

        return predecessors;
    }

    std::vector<Endpoint*> NetlistAbstraction::get_predecessors(const Endpoint* endpoint) const
    {
        return m_predecessors.at(endpoint);
    }

    std::vector<Gate*> NetlistAbstraction::get_unique_predecessors(const Gate* gate) const
    {
        std::vector<Gate*> predecessors;
        for (auto* ep : gate->get_fan_out_endpoints())
        {
            for (const auto* pred_ep : m_predecessors.at(ep))
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
        for (const auto* pred_ep : m_predecessors.at(endpoint))
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
            const auto new_successors = m_successors.at(ep);
            successors.insert(successors.end(), new_successors.begin(), new_successors.end());
        }

        return successors;
    }

    std::vector<Endpoint*> NetlistAbstraction::get_successors(const Endpoint* endpoint) const
    {
        return m_successors.at(endpoint);
    }

    std::vector<Gate*> NetlistAbstraction::get_unique_successors(const Gate* gate) const
    {
        std::vector<Gate*> successors;
        for (auto* ep : gate->get_fan_in_endpoints())
        {
            for (const auto* succ_ep : m_successors.at(ep))
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
        for (const auto* succ_ep : m_successors.at(endpoint))
        {
            successors.push_back(succ_ep->get_gate());
        }

        std::sort(successors.begin(), successors.end());
        successors.erase(std::unique(successors.begin(), successors.end()), successors.end());

        return successors;
    }

    std::vector<Net*> NetlistAbstraction::get_global_input_predecessors(const Endpoint* endpoint) const
    {
        return m_global_input_predecessors.at(endpoint);
    }

    std::vector<Net*> NetlistAbstraction::get_global_output_successors(const Endpoint* endpoint) const
    {
        return m_global_output_successors.at(endpoint);
    }

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance_internal(const std::vector<Endpoint*> start,
                                                                                                const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                                                const PinDirection& direction,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (direction == PinDirection::output || direction == PinDirection::input)
        {
            u32 distance = 0;

            // check whether start already fullfills target or exit filter
            for (auto* start_ep : start)
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(start_ep, distance))
                {
                    continue;
                    ;
                }

                if (target_filter(start_ep, m_abstraction))
                {
                    return OK(distance);
                }
            }

            std::unordered_set<Endpoint*> visited;

            std::vector<Endpoint*> current = start;
            std::vector<Endpoint*> next;

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
            const auto res_backward = get_shortest_path_distance_internal(start, target_filter, PinDirection::input);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_shortest_path_distance_internal(start, target_filter, PinDirection::output);
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
}    // namespace hal