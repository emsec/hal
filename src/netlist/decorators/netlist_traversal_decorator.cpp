#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetlistTraversalDecorator::NetlistTraversalDecorator(const Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::unordered_set<Gate*>> NetlistTraversalDecorator::get_next_gates(const Net* net,
                                                                                bool successors,
                                                                                const std::function<bool(const Gate*)>& filter,
                                                                                std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache) const
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

    Result<std::unordered_set<Gate*>> NetlistTraversalDecorator::get_next_gates(const Gate* gate,
                                                                                bool successors,
                                                                                const std::function<bool(const Gate*)>& filter,
                                                                                std::unordered_map<const Net*, std::unordered_set<Gate*>>& cache) const
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
        for (const auto& n : successors ? gate->get_fan_out_nets() : gate->get_fan_in_nets())
        {
            const auto next_res = this->get_next_gates(n, successors, filter, cache);
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