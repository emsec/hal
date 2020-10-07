#include "hal_core/netlist/grouping.h"

#include "hal_core/netlist/event_system/grouping_event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    Grouping::Grouping(NetlistInternalManager* internal_manager, u32 id, std::string name)
    {
        m_id               = id;
        m_name             = name;
        m_internal_manager = internal_manager;
    }

    u32 Grouping::get_id()
    {
        return m_id;
    }

    void Grouping::set_name(std::string name)
    {
        m_name = name;
    }

    std::string Grouping::get_name()
    {
        return m_name;
    }

    bool Grouping::assign_gate(Gate* gate)
    {
        u32 gate_id = gate->get_id();

        if (m_gates_map.find(gate_id) != m_gates_map.end())
        {
            log_error("grouping", "cannot assign gate with ID {:08x} as gate is already part of grouping with ID {:08x}.", gate_id, m_id);
            return false;
        }

        m_gates.push_back(gate);
        m_gates_map.emplace(gate_id, gate);

        grouping_event_handler::notify(grouping_event_handler::event::gate_assigned, this, gate_id);

        return true;
    }

    bool Grouping::assign_gate_by_id(const u32 gate_id)
    {
        Gate* gate = m_internal_manager->m_netlist->get_gate_by_id(gate_id);
        if (gate == nullptr)
        {
            return false;
        }

        return assign_gate(gate);
    }

    std::vector<Gate*> Grouping::get_gates()
    {
        return m_gates;
    }

    std::vector<u32> Grouping::get_gate_ids()
    {
        std::vector<u32> gate_ids(m_gates.size());

        for (const Gate* const gate : m_gates)
        {
            gate_ids.push_back(gate->get_id());
        }

        return gate_ids;
    }

    bool Grouping::remove_gate(Gate* gate)
    {
        u32 gate_id = gate->get_id();

        if (auto map_it = m_gates_map.find(gate_id); map_it != m_gates_map.end())
        {
            log_error("grouping", "cannot remove gate with ID {:08x} as gate is not part of grouping with ID {:08x}.", gate_id, m_id);
            return false;
        }
        else
        {
            auto vec_it = std::find(m_gates.begin(), m_gates.end(), gate);

            *vec_it = m_gates.back();
            m_gates.pop_back();
            m_gates_map.erase(gate_id);
        }

        grouping_event_handler::notify(grouping_event_handler::event::gate_removed, this, gate_id);

        return true;
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

    bool Grouping::contains_gate(Gate* gate)
    {
        return contains_gate_by_id(gate->get_id());
    }

    bool Grouping::contains_gate_by_id(const u32 gate_id)
    {
        return m_gates_map.find(gate_id) != m_gates_map.end();
    }

    bool Grouping::assign_net(Net* net)
    {
        u32 net_id = net->get_id();

        if (m_nets_map.find(net_id) != m_nets_map.end())
        {
            log_error("grouping", "cannot assign net with ID {:08x} as net is already part of grouping with ID {:08x}.", net_id, m_id);
            return false;
        }

        m_nets.push_back(net);
        m_nets_map.emplace(net_id, net);

        grouping_event_handler::notify(grouping_event_handler::event::net_assigned, this, net_id);

        return true;
    }

    bool Grouping::assign_net_by_id(const u32 net_id)
    {
        Net* net = m_internal_manager->m_netlist->get_net_by_id(net_id);
        if (net == nullptr)
        {
            return false;
        }

        return assign_net(net);
    }

    std::vector<Net*> Grouping::get_nets()
    {
        return m_nets;
    }

    std::vector<u32> Grouping::get_net_ids()
    {
        std::vector<u32> net_ids(m_nets.size());

        for (const Net* const net : m_nets)
        {
            net_ids.push_back(net->get_id());
        }

        return net_ids;
    }

    bool Grouping::remove_net(Net* net)
    {
        u32 net_id = net->get_id();

        if (auto map_it = m_nets_map.find(net_id); map_it != m_nets_map.end())
        {
            log_error("grouping", "cannot remove net with ID {:08x} as net is not part of grouping with ID {:08x}.", net_id, m_id);
            return false;
        }
        else
        {
            auto vec_it = std::find(m_nets.begin(), m_nets.end(), net);

            *vec_it = m_nets.back();
            m_nets.pop_back();
            m_nets_map.erase(net_id);
        }

        grouping_event_handler::notify(grouping_event_handler::event::net_removed, this, net_id);

        return true;
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

    bool Grouping::contains_net(Net* net)
    {
        return contains_net_by_id(net->get_id());
    }

    bool Grouping::contains_net_by_id(const u32 net_id)
    {
        return m_nets_map.find(net_id) != m_nets_map.end();
    }

    bool Grouping::assign_module(Module* module)
    {
        u32 module_id = module->get_id();

        if (m_modules_map.find(module_id) != m_modules_map.end())
        {
            log_error("grouping", "cannot assign module with ID {:08x} as module is already part of grouping with ID {:08x}.", module_id, m_id);
            return false;
        }

        m_modules.push_back(module);
        m_modules_map.emplace(module_id, module);

        grouping_event_handler::notify(grouping_event_handler::event::module_assigned, this, module_id);

        return true;
    }

    bool Grouping::assign_module_by_id(const u32 module_id)
    {
        Module* module = m_internal_manager->m_netlist->get_module_by_id(module_id);
        if (module == nullptr)
        {
            return false;
        }

        return assign_module(module);
    }

    std::vector<Module*> Grouping::get_modules()
    {
        return m_modules;
    }

    std::vector<u32> Grouping::get_module_ids()
    {
        std::vector<u32> module_ids(m_modules.size());

        for (const Module* const module : m_modules)
        {
            module_ids.push_back(module->get_id());
        }

        return module_ids;
    }

    bool Grouping::remove_module(Module* module)
    {
        u32 module_id = module->get_id();

        if (auto map_it = m_modules_map.find(module_id); map_it != m_modules_map.end())
        {
            log_error("grouping", "cannot remove module with ID {:08x} as module is not part of grouping with ID {:08x}.", module_id, m_id);
            return false;
        }
        else
        {
            auto vec_it = std::find(m_modules.begin(), m_modules.end(), module);

            *vec_it = m_modules.back();
            m_modules.pop_back();
            m_modules_map.erase(module_id);
        }

        grouping_event_handler::notify(grouping_event_handler::event::module_removed, this, module_id);

        return true;
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

    bool Grouping::contains_module(Module* module)
    {
        return contains_module_by_id(module->get_id());
    }

    bool Grouping::contains_module_by_id(const u32 module_id)
    {
        return m_modules_map.find(module_id) != m_modules_map.end();
    }
}    // namespace hal