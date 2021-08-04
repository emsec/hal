#include "hal_core/netlist/grouping.h"

#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    Grouping::Grouping(NetlistInternalManager* internal_manager, EventHandler* event_handler, u32 id, std::string name)
    {
        m_id               = id;
        m_name             = name;
        m_internal_manager = internal_manager;

        m_event_handler    = event_handler;
    }

    u32 Grouping::get_id() const
    {
        return m_id;
    }

    void Grouping::set_name(std::string name)
    {
        if (utils::trim(name).empty())
        {
            log_error("grouping", "grouping name cannot be empty.");
            return;
        }
        if (name != m_name)
        {
            m_name = name;

            m_event_handler->notify(GroupingEvent::event::name_changed, this);
        }
    }

    std::string Grouping::get_name() const
    {
        return m_name;
    }

    Netlist* Grouping::get_netlist() const
    {
        return m_internal_manager->m_netlist;
    }

    bool Grouping::assign_gate(Gate* gate, bool force)
    {
        return m_internal_manager->grouping_assign_gate(this, gate, force);
    }

    bool Grouping::assign_gate_by_id(const u32 gate_id, bool force)
    {
        Gate* gate = m_internal_manager->m_netlist->get_gate_by_id(gate_id);
        if (gate == nullptr)
        {
            return false;
        }

        return assign_gate(gate, force);
    }

    std::vector<Gate*> Grouping::get_gates(const std::function<bool(Gate*)>& filter) const
    {
        std::vector<Gate*> res;

        if (!filter)
        {
            res = m_gates;
        }
        else
        {
            for (Gate* gate : m_gates)
            {
                if (!filter(gate))
                {
                    continue;
                }
                res.push_back(gate);
            }
        }

        return res;
    }

    std::vector<u32> Grouping::get_gate_ids(const std::function<bool(Gate*)>& filter) const
    {
        std::vector<u32> gate_ids;

        for (const Gate* const gate : get_gates(filter))
        {
            gate_ids.push_back(gate->get_id());
        }

        return gate_ids;
    }

    bool Grouping::remove_gate(Gate* gate)
    {
        return m_internal_manager->grouping_remove_gate(this, gate);
    }

    bool Grouping::remove_gate_by_id(const u32 gate_id)
    {
        Gate* gate = m_internal_manager->m_netlist->get_gate_by_id(gate_id);
        if (gate == nullptr)
        {
            return false;
        }

        return remove_gate(gate);
    }

    bool Grouping::contains_gate(Gate* gate) const
    {
        if (gate == nullptr)
        {
            return false;
        }

        return contains_gate_by_id(gate->get_id());
    }

    bool Grouping::contains_gate_by_id(const u32 gate_id) const
    {
        return m_gates_map.find(gate_id) != m_gates_map.end();
    }

    bool Grouping::assign_net(Net* net, bool force)
    {
        return m_internal_manager->grouping_assign_net(this, net, force);
    }

    bool Grouping::assign_net_by_id(const u32 net_id, bool force)
    {
        Net* net = m_internal_manager->m_netlist->get_net_by_id(net_id);
        if (net == nullptr)
        {
            return false;
        }

        return assign_net(net, force);
    }

    std::vector<Net*> Grouping::get_nets(const std::function<bool(Net*)>& filter) const
    {
        std::vector<Net*> res;

        if (!filter)
        {
            res = m_nets;
        }
        else
        {
            for (Net* net : m_nets)
            {
                if (!filter(net))
                {
                    continue;
                }
                res.push_back(net);
            }
        }

        return res;
    }

    std::vector<u32> Grouping::get_net_ids(const std::function<bool(Net*)>& filter) const
    {
        std::vector<u32> net_ids;

        for (const Net* const net : get_nets(filter))
        {
            net_ids.push_back(net->get_id());
        }

        return net_ids;
    }

    bool Grouping::remove_net(Net* net)
    {
        return m_internal_manager->grouping_remove_net(this, net);
    }

    bool Grouping::remove_net_by_id(const u32 net_id)
    {
        Net* net = m_internal_manager->m_netlist->get_net_by_id(net_id);
        if (net == nullptr)
        {
            return false;
        }

        return remove_net(net);
    }

    bool Grouping::contains_net(Net* net) const
    {
        if (net == nullptr)
        {
            return false;
        }

        return contains_net_by_id(net->get_id());
    }

    bool Grouping::contains_net_by_id(const u32 net_id) const
    {
        return m_nets_map.find(net_id) != m_nets_map.end();
    }

    bool Grouping::assign_module(Module* module, bool force)
    {
        return m_internal_manager->grouping_assign_module(this, module, force);
    }

    bool Grouping::assign_module_by_id(const u32 module_id, bool force)
    {
        Module* module = m_internal_manager->m_netlist->get_module_by_id(module_id);
        if (module == nullptr)
        {
            return false;
        }

        return assign_module(module, force);
    }

    std::vector<Module*> Grouping::get_modules(const std::function<bool(Module*)>& filter) const
    {
        std::vector<Module*> res;

        if (!filter)
        {
            res = m_modules;
        }
        else
        {
            for (Module* module : m_modules)
            {
                if (!filter(module))
                {
                    continue;
                }
                res.push_back(module);
            }
        }

        return res;
    }

    std::vector<u32> Grouping::get_module_ids(const std::function<bool(Module*)>& filter) const
    {
        std::vector<u32> module_ids;

        for (const Module* const module : get_modules(filter))
        {
            module_ids.push_back(module->get_id());
        }

        return module_ids;
    }

    bool Grouping::remove_module(Module* module)
    {
        return m_internal_manager->grouping_remove_module(this, module);
    }

    bool Grouping::remove_module_by_id(const u32 module_id)
    {
        Module* module = m_internal_manager->m_netlist->get_module_by_id(module_id);
        if (module == nullptr)
        {
            return false;
        }

        return remove_module(module);
    }

    bool Grouping::contains_module(Module* module) const
    {
        if (module == nullptr)
        {
            return false;
        }

        return contains_module_by_id(module->get_id());
    }

    bool Grouping::contains_module_by_id(const u32 module_id) const
    {
        return m_modules_map.find(module_id) != m_modules_map.end();
    }
}    // namespace hal
