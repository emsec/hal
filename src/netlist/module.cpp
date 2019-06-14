#include "netlist/module.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_internal_manager.h"

#include "netlist/event_system/module_event_handler.h"

#include "core/log.h"

module::module(u32 id, std::shared_ptr<module> parent, const std::string& name, netlist_internal_manager* internal_manager)
{
    m_internal_manager = internal_manager;
    m_id               = id;
    m_parent           = parent;
    m_name             = name;
}

u32 module::get_id() const
{
    return m_id;
}

std::string module::get_name() const
{
    return m_name;
}

void module::set_name(const std::string& name)
{
    if (core_utils::trim(name).empty())
    {
        log_error("module", "empty name is not allowed");
        return;
    }
    if (name != m_name)
    {
        m_name = name;

        module_event_handler::notify(module_event_handler::event::name_changed, shared_from_this());
    }
}

std::shared_ptr<module> module::get_parent_module() const
{
    return m_parent;
}

bool module::set_parent_module(const std::shared_ptr<module>& new_parent)
{
    if (new_parent == m_parent)
    {
        log_error("module", "can not set module as its own parent");
        return false;
    }

    if (m_parent == nullptr)
    {
        log_error("module", "no parent can be assigned to the top module");
        return false;
    }

    auto children = get_submodules(DONT_CARE, true);
    if (children.find(new_parent) != children.end())
    {
        new_parent->set_parent_module(m_parent);
    }

    m_parent->m_submodules_map.erase(m_id);
    m_parent->m_submodules_set.erase(shared_from_this());

    module_event_handler::notify(module_event_handler::event::submodule_removed, m_parent, m_id);

    m_parent = new_parent;

    m_parent->m_submodules_map[m_id] = shared_from_this();
    m_parent->m_submodules_set.insert(shared_from_this());

    module_event_handler::notify(module_event_handler::event::parent_changed, shared_from_this());
    module_event_handler::notify(module_event_handler::event::submodule_added, m_parent, m_id);

    return true;
}

std::set<std::shared_ptr<module>> module::get_submodules(const std::string& name_filter, bool recursive) const
{
    std::set<std::shared_ptr<module>> res;
    if (name_filter == DONT_CARE)
    {
        res = m_submodules_set;
    }
    else
    {
        for (const auto& sm : m_submodules_set)
        {
            if (sm->m_name == name_filter)
            {
                res.insert(sm);
            }
        }
    }

    if (recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            auto more = sm->get_submodules(name_filter, true);
            res.insert(more.begin(), more.end());
        }
    }
    return res;
}

std::shared_ptr<netlist> module::get_netlist() const
{
    return m_internal_manager->m_netlist->get_shared();
}

bool module::assign_gate(std::shared_ptr<gate> gate)
{
    if (gate == nullptr)
    {
        return false;
    }
    if (contains_gate(gate))
    {
        return false;
    }
    auto prev_module            = m_internal_manager->remove_from_submodules(gate);
    m_gates_map[gate->get_id()] = gate;
    m_gates_set.insert(gate);

    module_event_handler::notify(module_event_handler::event::gate_removed, prev_module, gate->get_id());
    module_event_handler::notify(module_event_handler::event::gate_assigned, shared_from_this(), gate->get_id());
    return true;
}

bool module::remove_gate(std::shared_ptr<gate> gate)
{
    if (gate == nullptr)
    {
        return false;
    }

    auto top = m_internal_manager->m_netlist->get_top_module();
    if (top.get() == this)
    {
        log_error("module", "cannot remove gates from top module.", gate->get_name(), gate->get_id(), m_name, m_id);
        return false;
    }

    auto it = m_gates_map.find(gate->get_id());

    if (it == m_gates_map.end())
    {
        log_error("module", "gate '{}' (id {}) is not stored in module '{}' (id {}).", gate->get_name(), gate->get_id(), m_name, m_id);
        return false;
    }

    m_gates_map.erase(it);
    m_gates_set.erase(gate);

    top->m_gates_map[gate->get_id()] = gate;
    top->m_gates_set.insert(gate);
    module_event_handler::notify(module_event_handler::event::gate_removed, shared_from_this(), gate->get_id());
    module_event_handler::notify(module_event_handler::event::gate_assigned, top, gate->get_id());

    return true;
}

bool module::contains_gate(std::shared_ptr<gate> const gate, bool recursive) const
{
    if (gate == nullptr)
    {
        return false;
    }
    bool success = (m_gates_set.find(gate) != m_gates_set.end());
    if (!success && recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            if (sm->contains_gate(gate, true))
            {
                return true;
            }
        }
    }
    return success;
}

std::shared_ptr<gate> module::get_gate_by_id(const u32 gate_id, bool recursive) const
{
    auto it = m_gates_map.find(gate_id);
    if (it == m_gates_map.end())
    {
        if (recursive)
        {
            for (const auto& sm : m_submodules_set)
            {
                auto res = sm->get_gate_by_id(gate_id, true);
                if (res != nullptr)
                {
                    return res;
                }
            }
        }
        return nullptr;
    }
    return it->second;
}

std::set<std::shared_ptr<gate>> module::get_gates(const std::string& gate_type_filter, const std::string& name_filter, bool recursive) const
{
    std::set<std::shared_ptr<gate>> res;
    if (gate_type_filter == DONT_CARE && name_filter == DONT_CARE)
    {
        res = m_gates_set;
    }
    else
    {
        for (const auto& it : m_gates_map)
        {
            auto current_gate = it.second;
            if ((gate_type_filter != DONT_CARE) && (current_gate->get_type() != gate_type_filter))
            {
                log_debug(
                    "module", "type of gate '{}' (id = {:08x}, type: '{}') does not match type '{}'.", current_gate->get_name(), current_gate->get_id(), current_gate->get_type(), gate_type_filter);
                continue;
            }
            if ((name_filter != DONT_CARE) && (current_gate->get_name() != name_filter))
            {
                log_debug("module", "name of gate '{}' (id = {:08x}, type: '{}') does not match name '{}'.", current_gate->get_name(), current_gate->get_id(), current_gate->get_type(), name_filter);
                continue;
            }
            res.insert(current_gate);
        }
    }

    if (recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            auto more = sm->get_gates(gate_type_filter, name_filter, true);
            res.insert(more.begin(), more.end());
        }
    }

    return res;
}

bool module::assign_net(std::shared_ptr<net> net)
{
    if (net == nullptr)
    {
        return false;
    }
    if (contains_net(net))
    {
        return false;
    }
    auto prev_module        = m_internal_manager->remove_from_submodules(net);
    m_nets_map[net->get_id()] = net;
    m_nets_set.insert(net);

    module_event_handler::notify(module_event_handler::event::net_removed, prev_module, net->get_id());
    module_event_handler::notify(module_event_handler::event::net_assigned, shared_from_this(), net->get_id());
    return true;
}

bool module::remove_net(std::shared_ptr<net> net)
{
    if (net == nullptr)
    {
        return false;
    }

    auto top = m_internal_manager->m_netlist->get_top_module();
    if (top.get() == this)
    {
        log_error("module", "cannot remove nets from top module.", net->get_name(), net->get_id(), m_name, m_id);
        return false;
    }

    auto it = m_nets_map.find(net->get_id());

    if (it == m_nets_map.end())
    {
        log_error("module", "net '{}' (id {}) is not stored in module '{}' (id {}).", net->get_name(), net->get_id(), m_name, m_id);
        return false;
    }

    m_nets_map.erase(it);
    m_nets_set.erase(net);

    top->m_nets_map[net->get_id()] = net;
    top->m_nets_set.insert(net);
    module_event_handler::notify(module_event_handler::event::net_removed, shared_from_this(), net->get_id());
    module_event_handler::notify(module_event_handler::event::net_assigned, top, net->get_id());

    return true;
}

bool module::contains_net(std::shared_ptr<net> const net, bool recursive) const
{
    if (net == nullptr)
    {
        return false;
    }
    bool success = (m_nets_set.find(net) != m_nets_set.end());
    if (!success && recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            if (sm->contains_net(net, true))
            {
                return true;
            }
        }
    }
    return success;
}

std::shared_ptr<net> module::get_net_by_id(const u32 net_id, bool recursive) const
{
    auto it = m_nets_map.find(net_id);
    if (it == m_nets_map.end())
    {
        if (recursive)
        {
            for (const auto& sm : m_submodules_set)
            {
                auto res = sm->get_net_by_id(net_id, true);
                if (res != nullptr)
                {
                    return res;
                }
            }
        }
        log_error("module", "no net with id = {} stored in module with id {}.", net_id, m_id);
        return nullptr;
    }
    return it->second;
}

std::set<std::shared_ptr<net>> module::get_nets(const std::string& name_filter, bool recursive) const
{
    std::set<std::shared_ptr<net>> res;
    if (name_filter == DONT_CARE)
    {
        res = m_nets_set;
    }
    else
    {
        for (const auto& net : m_nets_set)
        {
            if (net->get_name() == name_filter)
            {
                res.insert(net);
            }
        }
    }

    if (recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            auto more = sm->get_nets(name_filter, true);
            res.insert(more.begin(), more.end());
        }
    }
    return res;
}

std::set<std::shared_ptr<net>> module::get_input_nets(const std::string& name_filter) const
{
    std::set<std::shared_ptr<net>> res;
    auto gates = get_gates(DONT_CARE, DONT_CARE, true);
    std::set<std::shared_ptr<net>> seen;
    for (const auto& gate : gates)
    {
        for (const auto& net : gate->get_fan_in_nets())
        {
            if (name_filter == DONT_CARE || net->get_name() == name_filter)
            {
                if (seen.find(net) != seen.end())
                {
                    continue;
                }
                seen.insert(net);
                if (m_internal_manager->m_netlist->is_global_input_net(net))
                {
                    res.insert(net);
                }
                else if (gates.find(net->get_src().gate) == gates.end())
                {
                    res.insert(net);
                }
            }
        }
    }
    return res;
}

std::set<std::shared_ptr<net>> module::get_output_nets(const std::string& name_filter) const
{
    std::set<std::shared_ptr<net>> res;
    auto gates = get_gates(DONT_CARE, DONT_CARE, true);
    std::set<std::shared_ptr<net>> seen;
    for (const auto& gate : gates)
    {
        for (const auto& net : gate->get_fan_out_nets())
        {
            if (name_filter == DONT_CARE || net->get_name() == name_filter)
            {
                if (seen.find(net) != seen.end())
                {
                    continue;
                }
                seen.insert(net);
                if (m_internal_manager->m_netlist->is_global_output_net(net))
                {
                    res.insert(net);
                    continue;
                }
                for (const auto& dst : net->get_dsts())
                {
                    if (gates.find(dst.gate) == gates.end())
                    {
                        res.insert(net);
                        break;
                    }
                }
            }
        }
    }
    return res;
}
