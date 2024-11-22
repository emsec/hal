

#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    Result<NetlistAbstraction> NetlistAbstraction::create(const Netlist* netlist,
                                                          const std::vector<Gate*>& gates,
                                                          const bool include_all_netlist_gates,
                                                          const std::function<bool(const Endpoint*, const u32 current_depth)>& exit_endpoint_filter,
                                                          const std::function<bool(const Endpoint*, const u32 current_depth)>& entry_endpoint_filter)
    {
        const auto nl_trav_dec = NetlistTraversalDecorator(*netlist);

        // transform gates into set to check fast if a gate is part of abstraction
        const auto gates_set = utils::to_unordered_set(gates);

        auto new_abstraction = NetlistAbstraction();

        for (const Gate* gate : include_all_netlist_gates ? netlist->get_gates() : gates)
        {
            // TODO remove debug print
            // std::cout << gate->get_id() << std::endl;

            // gather all successors
            for (Endpoint* ep_out : gate->get_fan_out_endpoints())
            {
                new_abstraction.m_successors.insert({ep_out, {}});
                const auto successors = nl_trav_dec.get_next_matching_endpoints(
                    ep_out,
                    true,
                    [gates_set](const auto& ep) { return ep->is_destination_pin() && gates_set.find(ep->get_gate()) != gates_set.end(); },
                    false,
                    exit_endpoint_filter,
                    entry_endpoint_filter);

                if (successors.is_error())
                {
                    return ERR_APPEND(successors.get_error(),
                                      "cannot build netlist abstraction: failed to gather succesor endpoints for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
                }

                for (Endpoint* ep : successors.get())
                {
                    new_abstraction.m_successors.at(ep_out).push_back(ep);
                }
            }

            // gather all global output succesors
            for (Endpoint* ep_out : gate->get_fan_out_endpoints())
            {
                new_abstraction.m_global_output_successors.insert({ep_out, {}});

                const auto destinations = nl_trav_dec.get_next_matching_endpoints(
                    ep_out, true, [](const auto& ep) { return ep->is_source_pin() && ep->get_net()->is_global_output_net(); }, false, exit_endpoint_filter, entry_endpoint_filter);

                if (destinations.is_error())
                {
                    return ERR_APPEND(destinations.get_error(),
                                      "cannot build netlist abstraction: failed to gather global succesor endpoints for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
                }

                for (const auto* ep : destinations.get())
                {
                    new_abstraction.m_global_output_successors.at(ep_out).push_back({ep->get_net()});
                }
            }

            // gather all predecessors
            for (Endpoint* ep_in : gate->get_fan_in_endpoints())
            {
                new_abstraction.m_predecessors.insert({ep_in, {}});

                const auto predecessors =
                    nl_trav_dec.get_next_matching_endpoints(ep_in, false, [gates_set](const auto& ep) { return ep->is_source_pin() && gates_set.find(ep->get_gate()) != gates_set.end(); });

                if (predecessors.is_error())
                {
                    return ERR_APPEND(predecessors.get_error(),
                                      "cannot build netlist abstraction: failed to gather predecessor endpoints for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
                }

                for (Endpoint* ep : predecessors.get())
                {
                    new_abstraction.m_predecessors.at(ep_in).push_back(ep);
                }
            }

            // gather all global input predecessors
            for (Endpoint* ep_in : gate->get_fan_in_endpoints())
            {
                new_abstraction.m_global_input_predecessors.insert({ep_in, {}});

                const auto predecessors = nl_trav_dec.get_next_matching_endpoints(ep_in, false, [](const auto& ep) { return ep->is_destination_pin() && ep->get_net()->is_global_input_net(); });

                if (predecessors.is_error())
                {
                    return ERR_APPEND(predecessors.get_error(),
                                      "cannot build netlist abstraction: failed to gather global predecessor endpoints for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()));
                }

                for (const auto* ep : predecessors.get())
                {
                    new_abstraction.m_global_input_predecessors.at(ep_in).push_back({ep->get_net()});
                }
            }
        }

        return OK(new_abstraction);
    }

    Result<std::vector<Endpoint*>> NetlistAbstraction::get_predecessors(const Gate* gate) const
    {
        std::vector<Endpoint*> predecessors;
        for (auto* ep : gate->get_fan_out_endpoints())
        {
            const auto new_predecessors = get_predecessors(ep);
            if (new_predecessors.is_error())
            {
                return ERR_APPEND(new_predecessors.get_error(), "failed to get predecessors of gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + " in netlist abstraction");
            }

            predecessors.insert(predecessors.end(), new_predecessors.get().begin(), new_predecessors.get().end());
        }

        return OK(predecessors);
    }

    Result<std::vector<Endpoint*>> NetlistAbstraction::get_predecessors(const Endpoint* endpoint) const
    {
        const auto it = m_predecessors.find(endpoint);
        if (it == m_predecessors.end())
        {
            return ERR("Found no predecessors for endpoint at gate " + endpoint->get_gate()->get_name() + " with ID " + std::to_string(endpoint->get_gate()->get_id()) + " and at pin "
                       + endpoint->get_pin()->get_name() + " in netlist abstraction");
        }
        return OK(it->second);
    }

    Result<std::vector<Gate*>> NetlistAbstraction::get_unique_predecessors(const Gate* gate) const
    {
        std::vector<Gate*> predecessors;
        for (auto* ep : gate->get_fan_out_endpoints())
        {
            const auto new_predecessors = get_predecessors(ep);
            if (new_predecessors.is_error())
            {
                return ERR_APPEND(new_predecessors.get_error(),
                                  "failed to get unique predecessors of gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + " in netlist abstraction");
            }

            for (const auto* pred_ep : new_predecessors.get())
            {
                predecessors.push_back(pred_ep->get_gate());
            }
        }

        std::sort(predecessors.begin(), predecessors.end());
        predecessors.erase(std::unique(predecessors.begin(), predecessors.end()), predecessors.end());

        return OK(predecessors);
    }

    Result<std::vector<Gate*>> NetlistAbstraction::get_unique_predecessors(const Endpoint* endpoint) const
    {
        std::vector<Gate*> predecessors;
        const auto new_predecessors = get_predecessors(endpoint);
        if (new_predecessors.is_error())
        {
            return ERR_APPEND(new_predecessors.get_error(),
                              "failed to get unique predecessors of endpoint at gate " + endpoint->get_gate()->get_name() + " with ID " + std::to_string(endpoint->get_gate()->get_id())
                                  + " and at pin " + endpoint->get_pin()->get_name() + " in netlist abstraction");
        }

        for (const auto* pred_ep : new_predecessors.get())
        {
            predecessors.push_back(pred_ep->get_gate());
        }

        std::sort(predecessors.begin(), predecessors.end());
        predecessors.erase(std::unique(predecessors.begin(), predecessors.end()), predecessors.end());

        return OK(predecessors);
    }

    Result<std::vector<Endpoint*>> NetlistAbstraction::get_successors(const Gate* gate) const
    {
        std::vector<Endpoint*> successors;
        for (auto* ep : gate->get_fan_in_endpoints())
        {
            const auto new_successors = get_predecessors(ep);
            if (new_successors.is_error())
            {
                return ERR_APPEND(new_successors.get_error(), "failed to get successors of gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + " in netlist abstraction");
            }

            successors.insert(successors.end(), new_successors.get().begin(), new_successors.get().end());
        }

        return OK(successors);
    }

    Result<std::vector<Endpoint*>> NetlistAbstraction::get_successors(const Endpoint* endpoint) const
    {
        const auto it = m_successors.find(endpoint);
        if (it == m_successors.end())
        {
            return ERR("Found no successors for endpoint at gate " + endpoint->get_gate()->get_name() + " with ID " + std::to_string(endpoint->get_gate()->get_id()) + " and at pin "
                       + endpoint->get_pin()->get_name() + " in netlist abstraction");
        }
        return OK(it->second);
    }

    Result<std::vector<Gate*>> NetlistAbstraction::get_unique_successors(const Gate* gate) const
    {
        std::vector<Gate*> successors;
        for (auto* ep : gate->get_fan_in_endpoints())
        {
            const auto new_successors = get_successors(ep);
            if (new_successors.is_error())
            {
                return ERR_APPEND(new_successors.get_error(), "failed to get unique successors of gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + " in netlist abstraction");
            }

            for (const auto* succ_ep : new_successors.get())
            {
                successors.push_back(succ_ep->get_gate());
            }
        }

        std::sort(successors.begin(), successors.end());
        successors.erase(std::unique(successors.begin(), successors.end()), successors.end());

        return OK(successors);
    }

    Result<std::vector<Gate*>> NetlistAbstraction::get_unique_successors(const Endpoint* endpoint) const
    {
        std::vector<Gate*> successors;
        const auto new_successors = get_successors(endpoint);
        if (new_successors.is_error())
        {
            return ERR_APPEND(new_successors.get_error(),
                              "failed to get unique successors of endpoint at gate " + endpoint->get_gate()->get_name() + " with ID " + std::to_string(endpoint->get_gate()->get_id()) + " and at pin "
                                  + endpoint->get_pin()->get_name() + " in netlist abstraction");
        }

        for (const auto* succ_ep : new_successors.get())
        {
            successors.push_back(succ_ep->get_gate());
        }

        std::sort(successors.begin(), successors.end());
        successors.erase(std::unique(successors.begin(), successors.end()), successors.end());

        return OK(successors);
    }

    Result<std::vector<Net*>> NetlistAbstraction::get_global_input_predecessors(const Endpoint* endpoint) const
    {
        const auto it = m_global_input_predecessors.find(endpoint);
        if (it == m_global_input_predecessors.end())
        {
            return ERR("found no global input predecessors of endpoint at gate " + endpoint->get_gate()->get_name() + " with ID " + std::to_string(endpoint->get_gate()->get_id()) + " and at pin "
                       + endpoint->get_pin()->get_name() + " in netlist abstraction");
        }

        return OK(it->second);
    }

    Result<std::vector<Net*>> NetlistAbstraction::get_global_output_successors(const Endpoint* endpoint) const
    {
        const auto it = m_global_output_successors.find(endpoint);
        if (it == m_global_output_successors.end())
        {
            return ERR("found no global output successors of endpoint at gate " + endpoint->get_gate()->get_name() + " with ID " + std::to_string(endpoint->get_gate()->get_id()) + " and at pin "
                       + endpoint->get_pin()->get_name() + " in netlist abstraction");
        }
        return OK(it->second);
    }

    NetlistAbstractionDecorator::NetlistAbstractionDecorator(const hal::NetlistAbstraction& abstraction) : m_abstraction(abstraction){};

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance_internal(const std::vector<Endpoint*>& start,
                                                                                                const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                                                const PinDirection& direction,
                                                                                                const bool directed,
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
                    return OK(std::optional<u32>(distance));
                }

                current.push_back(start_ep);
                visited.insert(start_ep);
            }

            while (true)
            {
                distance++;

                for (const auto& exit_ep : current)
                {
                    // currently only works for input and output pins
                    if (exit_ep->get_pin()->get_direction() != PinDirection::output && exit_ep->get_pin()->get_direction() != PinDirection::input)
                    {
                        return ERR("failed to get shortest path distance: found endpoint at gate " + exit_ep->get_gate()->get_name() + " with ID " + std::to_string(exit_ep->get_gate()->get_id())
                                   + " and pin " + exit_ep->get_pin()->get_name() + " with direction " + enum_to_string(exit_ep->get_pin()->get_direction()) + " that is currently unhandled");
                    }

                    const auto entry_eps = (exit_ep->get_pin()->get_direction() == PinDirection::output) ? m_abstraction.get_successors(exit_ep) : m_abstraction.get_predecessors(exit_ep);
                    if (entry_eps.is_error())
                    {
                        return ERR_APPEND(entry_eps.get_error(), "cannot get shortest path distance starting from endpoints");
                    }

                    for (const auto& entry_ep : entry_eps.get())
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

                        std::vector<Endpoint*> next_eps;
                        if (directed)
                        {
                            next_eps = (direction == PinDirection::output) ? next_g->get_fan_out_endpoints() : next_g->get_fan_in_endpoints();
                        }
                        else
                        {
                            next_eps.insert(next_eps.end(), next_g->get_fan_out_endpoints().begin(), next_g->get_fan_out_endpoints().end());
                            next_eps.insert(next_eps.end(), next_g->get_fan_in_endpoints().begin(), next_g->get_fan_in_endpoints().end());
                        }

                        for (const auto& next_ep : next_eps)
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

            return OK(std::nullopt);
        }

        if (direction == PinDirection::inout)
        {
            const auto res_backward = get_shortest_path_distance_internal(start, target_filter, PinDirection::input, directed, exit_endpoint_filter, entry_endpoint_filter);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_shortest_path_distance_internal(start, target_filter, PinDirection::output, directed, exit_endpoint_filter, entry_endpoint_filter);
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
                                                                                       const bool directed,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (start->get_pin()->get_direction() != direction)
        {
            return get_shortest_path_distance(start->get_gate(), target_filter, direction, directed, exit_endpoint_filter, entry_endpoint_filter);
        }

        return get_shortest_path_distance_internal({start}, target_filter, direction, true, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance(const Gate* start,
                                                                                       const std::function<bool(const Endpoint*, const NetlistAbstraction&)>& target_filter,
                                                                                       const PinDirection& direction,
                                                                                       const bool directed,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                       const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        std::vector<Endpoint*> start_endpoints;
        if (direction == PinDirection::output || direction == PinDirection::inout)
        {
            for (auto* exit_ep : start->get_fan_out_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
                {
                    continue;
                }

                start_endpoints.push_back(exit_ep);
            }
        }

        if (direction == PinDirection::input || direction == PinDirection::inout)
        {
            for (auto* exit_ep : start->get_fan_in_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
                {
                    continue;
                }

                start_endpoints.push_back(exit_ep);
            }
        }

        return get_shortest_path_distance_internal(start_endpoints, target_filter, direction, directed, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::optional<u32>> NetlistAbstractionDecorator::get_shortest_path_distance(const Gate* start,
                                                                                       const Gate* target,
                                                                                       const PinDirection& direction,
                                                                                       const bool directed,
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
            directed,
            exit_endpoint_filter,
            entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_internal(const std::vector<Endpoint*>& start,
                                                                                          const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                          const PinDirection& direction,
                                                                                          const bool directed,
                                                                                          bool continue_on_match,
                                                                                          const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                          const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (direction == PinDirection::output || direction == PinDirection::input)
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
                visited.insert(start_ep);
            }

            while (true)
            {
                distance++;

                for (const auto& exit_ep : current)
                {
                    // currently only works for input and output pins
                    if (exit_ep->get_pin()->get_direction() != PinDirection::output && exit_ep->get_pin()->get_direction() != PinDirection::input)
                    {
                        return ERR("failed to get shortest path distance: found endpoint at gate " + exit_ep->get_gate()->get_name() + " with ID " + std::to_string(exit_ep->get_gate()->get_id())
                                   + " and pin " + exit_ep->get_pin()->get_name() + " with direction " + enum_to_string(exit_ep->get_pin()->get_direction()) + " that is currently unhandled");
                    }

                    const auto entry_eps = (exit_ep->get_pin()->get_direction() == PinDirection::output) ? m_abstraction.get_successors(exit_ep) : m_abstraction.get_predecessors(exit_ep);
                    if (entry_eps.is_error())
                    {
                        return ERR_APPEND(entry_eps.get_error(), "cannot get shortest path distance starting from endpoints");
                    }

                    for (const auto& entry_ep : entry_eps.get())
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

                        std::vector<Endpoint*> next_eps;
                        if (directed)
                        {
                            next_eps = (direction == PinDirection::output) ? next_g->get_fan_out_endpoints() : next_g->get_fan_in_endpoints();
                        }
                        else
                        {
                            next_eps.insert(next_eps.end(), next_g->get_fan_out_endpoints().begin(), next_g->get_fan_out_endpoints().end());
                            next_eps.insert(next_eps.end(), next_g->get_fan_in_endpoints().begin(), next_g->get_fan_in_endpoints().end());
                        }

                        for (const auto& next_ep : next_eps)
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

        if (direction == PinDirection::inout)
        {
            const auto res_backward = get_next_matching_gates_internal(start, target_gate_filter, PinDirection::input, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward = get_next_matching_gates_internal(start, target_gate_filter, PinDirection::output, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
            if (res_forward.is_error())
            {
                return res_forward;
            }

            const auto matching_gates_backward = res_backward.get();
            const auto matching_gates_forward  = res_forward.get();

            std::set<Gate*> matching_gates = matching_gates_backward;
            matching_gates.insert(matching_gates_forward.begin(), matching_gates_forward.end());

            return OK(matching_gates);
        }

        return ERR("cannot get next matching gates until: pin direction " + enum_to_string(direction) + " is not supported");
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates(Endpoint* endpoint,
                                                                                 const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                 const PinDirection& direction,
                                                                                 const bool directed,
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
            return get_next_matching_gates(endpoint->get_gate(), target_gate_filter, direction, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
        }

        return get_next_matching_gates_internal({endpoint}, target_gate_filter, direction, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates(const Gate* gate,
                                                                                 const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                 const PinDirection& direction,
                                                                                 const bool directed,
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
        if (direction == PinDirection::output || direction == PinDirection::inout)
        {
            for (auto* exit_ep : gate->get_fan_out_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
                {
                    continue;
                }

                start.push_back(exit_ep);
            }
        }

        if (direction == PinDirection::input || direction == PinDirection::inout)
        {
            for (auto* exit_ep : gate->get_fan_in_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
                {
                    continue;
                }

                start.push_back(exit_ep);
            }
        }

        return get_next_matching_gates_internal(start, target_gate_filter, direction, directed, continue_on_match, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_until_internal(const std::vector<Endpoint*>& start,
                                                                                                const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                                const PinDirection& direction,
                                                                                                const bool directed,
                                                                                                bool continue_on_mismatch,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& exit_endpoint_filter,
                                                                                                const std::function<bool(const Endpoint*, u32 current_depth)>& entry_endpoint_filter) const
    {
        if (direction == PinDirection::output || direction == PinDirection::input)
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
                visited.insert(start_ep);
            }

            while (true)
            {
                distance++;

                for (const auto& exit_ep : current)
                {
                    // currently only works for input and output pins
                    if (exit_ep->get_pin()->get_direction() != PinDirection::output && exit_ep->get_pin()->get_direction() != PinDirection::input)
                    {
                        return ERR("failed to get shortest path distance: found endpoint at gate " + exit_ep->get_gate()->get_name() + " with ID " + std::to_string(exit_ep->get_gate()->get_id())
                                   + " and pin " + exit_ep->get_pin()->get_name() + " with direction " + enum_to_string(exit_ep->get_pin()->get_direction()) + " that is currently unhandled");
                    }

                    const auto entry_eps = (exit_ep->get_pin()->get_direction() == PinDirection::output) ? m_abstraction.get_successors(exit_ep) : m_abstraction.get_predecessors(exit_ep);
                    if (entry_eps.is_error())
                    {
                        return ERR_APPEND(entry_eps.get_error(), "cannot get shortest path distance starting from endpoints");
                    }

                    for (const auto& entry_ep : entry_eps.get())
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

                        std::vector<Endpoint*> next_eps;
                        if (directed)
                        {
                            next_eps = (direction == PinDirection::output) ? next_g->get_fan_out_endpoints() : next_g->get_fan_in_endpoints();
                        }
                        else
                        {
                            next_eps.insert(next_eps.end(), next_g->get_fan_out_endpoints().begin(), next_g->get_fan_out_endpoints().end());
                            next_eps.insert(next_eps.end(), next_g->get_fan_in_endpoints().begin(), next_g->get_fan_in_endpoints().end());
                        }

                        for (const auto& next_ep : next_eps)
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

        if (direction == PinDirection::inout)
        {
            const auto res_backward =
                get_next_matching_gates_until_internal(start, target_gate_filter, PinDirection::input, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
            if (res_backward.is_error())
            {
                return res_backward;
            }

            const auto res_forward =
                get_next_matching_gates_until_internal(start, target_gate_filter, PinDirection::output, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
            if (res_forward.is_error())
            {
                return res_forward;
            }

            const auto matching_gates_backward = res_backward.get();
            const auto matching_gates_forward  = res_forward.get();

            std::set<Gate*> matching_gates = matching_gates_backward;
            matching_gates.insert(matching_gates_forward.begin(), matching_gates_forward.end());

            return OK(matching_gates);
        }

        return ERR("cannot get next matching gates until: pin direction " + enum_to_string(direction) + " is not supported");
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_until(Endpoint* endpoint,
                                                                                       const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                       const PinDirection& direction,
                                                                                       const bool directed,
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
            return get_next_matching_gates_until(endpoint->get_gate(), target_gate_filter, direction, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
        }

        return get_next_matching_gates_until_internal({endpoint}, target_gate_filter, direction, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
    }

    Result<std::set<Gate*>> NetlistAbstractionDecorator::get_next_matching_gates_until(const Gate* gate,
                                                                                       const std::function<bool(const Gate*)>& target_gate_filter,
                                                                                       const PinDirection& direction,
                                                                                       const bool directed,
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
        if (direction == PinDirection::output || direction == PinDirection::inout)
        {
            for (auto* exit_ep : gate->get_fan_out_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
                {
                    continue;
                }

                start.push_back(exit_ep);
            }
        }

        if (direction == PinDirection::input || direction == PinDirection::inout)
        {
            for (auto* exit_ep : gate->get_fan_in_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, 0))
                {
                    continue;
                }

                start.push_back(exit_ep);
            }
        }

        return get_next_matching_gates_internal(start, target_gate_filter, direction, directed, continue_on_mismatch, exit_endpoint_filter, entry_endpoint_filter);
    }

}    // namespace hal