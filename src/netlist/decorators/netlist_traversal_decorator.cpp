#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    NetlistTraversalDecorator::NetlistTraversalDecorator(const Netlist& netlist) : m_netlist(netlist)
    {
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_gates(const Net* net,
                                                                      bool successors,
                                                                      const std::function<bool(const Gate*)>& target_gate_filter,
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

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_next_gates(const Gate* gate,
                                                                      bool successors,
                                                                      const std::function<bool(const Gate*)>& target_gate_filter,
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

            const auto next_res = this->get_next_gates(exit_ep->get_net(), successors, target_gate_filter, exit_endpoint_filter, entry_endpoint_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_subgraph_gates(const Net* net,
                                                                          bool successors,
                                                                          const std::function<bool(const Gate*)>& target_gate_filter,
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

                if ((target_gate_filter == nullptr) || target_gate_filter(g))
                {
                    res.insert(g);
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

    Result<std::set<Gate*>> NetlistTraversalDecorator::get_subgraph_gates(const Gate* gate,
                                                                          bool successors,
                                                                          const std::function<bool(const Gate*)>& target_gate_filter,
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

            const auto next_res = this->get_subgraph_gates(exit_ep->get_net(), successors, target_gate_filter, exit_endpoint_filter, entry_endpoint_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

    Result<std::set<Net*>> NetlistTraversalDecorator::get_subgraph_input_nets(const Net* net,
                                                                              bool successors,
                                                                              const std::function<bool(const Net*)>& target_net_filter,
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

        std::set<Net*> res;
        for (const auto* entry_ep : successors ? net->get_destinations() : net->get_sources())
        {
            if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, 0))
            {
                continue;
            }

            const auto next_res = this->get_subgraph_input_nets(entry_ep->get_gate(), successors, target_net_filter, exit_endpoint_filter, entry_endpoint_filter);
            if (next_res.is_error())
            {
                return ERR(next_res.get_error());
            }
            auto next = next_res.get();
            res.insert(next.begin(), next.end());
        }
        return OK(res);
    }

    Result<std::set<Net*>> NetlistTraversalDecorator::get_subgraph_input_nets(const Gate* gate,
                                                                              bool successors,
                                                                              const std::function<bool(const Net*)>& target_net_filter,
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

        std::unordered_set<const Gate*> visited;
        std::vector<const Gate*> stack = {gate};
        std::vector<const Gate*> previous;
        std::set<Net*> res;
        while (!stack.empty())
        {
            const Gate* current = stack.back();

            if (!previous.empty() && current == previous.back())
            {
                stack.pop_back();
                previous.pop_back();
                continue;
            }

            visited.insert(current);

            bool added = false;
            for (const auto* exit_ep : successors ? current->get_fan_out_endpoints() : current->get_fan_in_endpoints())
            {
                if (exit_endpoint_filter != nullptr && !exit_endpoint_filter(exit_ep, previous.size()))
                {
                    continue;
                }

                auto* n = exit_ep->get_net();

                if (target_net_filter(n))
                {
                    res.insert(n);
                }
                else
                {
                    for (const auto* entry_ep : successors ? n->get_destinations() : n->get_sources())
                    {
                        if (entry_endpoint_filter != nullptr && !entry_endpoint_filter(entry_ep, previous.size() + 1))
                        {
                            continue;
                        }

                        const Gate* g = entry_ep->get_gate();
                        if (visited.find(g) == visited.end())
                        {
                            stack.push_back(g);
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

}    // namespace hal