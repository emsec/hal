#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetlistTraversalDecorator::NetlistTraversalDecorator(const Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::unordered_set<Gate*>> NetlistTraversalDecorator::get_next_gates(std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache,
                                                                                const Net* net,
                                                                                bool successors,
                                                                                const std::function<bool(const Gate*)>& filter,
                                                                                const std::set<PinType>& forbidden_pins) const
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

            if (const auto it = cache.find(current); it != cache.end())
            {
                for (const auto* n : previous)
                {
                    const std::unordered_set<Gate*>& cached_gates = std::get<1>(*it);
                    cache[n].insert(cached_gates.begin(), cached_gates.end());
                }
                stack.pop_back();
            }
            else
            {
                bool added = false;
                for (const auto* ep : successors ? current->get_destinations() : current->get_sources())
                {
                    if (!forbidden_pins.empty())
                    {
                        if (forbidden_pins.find(ep->get_pin()->get_type()) != forbidden_pins.end())
                        {
                            continue;
                        }
                    }

                    auto* g = ep->get_gate();

                    if (filter(g))
                    {
                        cache[current].insert(g);
                        for (const auto* n : previous)
                        {
                            cache[n].insert(g);
                        }
                    }
                    else
                    {
                        for (const auto* n : successors ? g->get_fan_out_nets() : g->get_fan_in_nets())
                        {
                            if (visited.find(n) == visited.end())
                            {
                                stack.push_back(n);
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
        }

        return OK(cache[net]);
    }

    Result<std::unordered_set<Gate*>> NetlistTraversalDecorator::get_next_gates(std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache,
                                                                                const Gate* gate,
                                                                                bool successors,
                                                                                const std::function<bool(const Gate*)>& filter,
                                                                                const std::set<PinType>& forbidden_pins) const
    {
        if (gate == nullptr)
        {
            return ERR("nullptr given as gate");
        }

        if (!m_netlist.is_gate_in_netlist(gate))
        {
            return ERR("net does not belong to netlist");
        }

        std::unordered_set<Gate*> res;
        for (const auto* ep : successors ? gate->get_fan_out_endpoints() : gate->get_fan_in_endpoints())
        {
            if (!forbidden_pins.empty())
            {
                if (forbidden_pins.find(ep->get_pin()->get_type()) != forbidden_pins.end())
                {
                    continue;
                }
            }

            const auto next_res = this->get_next_gates(cache, ep->get_net(), successors, filter, forbidden_pins);
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