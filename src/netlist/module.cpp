#include "netlist/module.h"

#include "core/log.h"
#include "netlist/event_system/module_event_handler.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_internal_manager.h"

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

std::string module::get_type() const
{
    return m_type;
}

void module::set_type(const std::string& type)
{
    if (core_utils::trim(type).empty())
    {
        log_error("module", "empty name is not allowed");
        return;
    }
    if (type != m_type)
    {
        m_type = type;

        module_event_handler::notify(module_event_handler::event::type_changed, shared_from_this());
    }
}

std::shared_ptr<module> module::get_parent_module() const
{
    return m_parent;
}

bool module::set_parent_module(const std::shared_ptr<module>& new_parent)
{
    if (new_parent == shared_from_this())
    {
        log_error("module", "can not set module as its own parent");
        return false;
    }

    if (m_parent == nullptr)
    {
        log_error("module", "no parent can be assigned to the top module");
        return false;
    }

    if (new_parent == nullptr)
    {
        log_error("module", "cannot reassign top module");
        return false;
    }

    if (!get_netlist()->is_module_in_netlist(new_parent))
    {
        log_error("module", "module must be in the current netlist");
        return false;
    }

    auto children = get_submodules(nullptr, true);
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

std::set<std::shared_ptr<module>> module::get_submodules(const std::function<bool(const std::shared_ptr<module>&)>& filter, bool recursive) const
{
    std::set<std::shared_ptr<module>> res;
    if (!filter)
    {
        res = m_submodules_set;
    }
    else
    {
        for (const auto& sm : m_submodules_set)
        {
            if (filter(sm))
            {
                res.insert(sm);
            }
        }
    }

    if (recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            auto more = sm->get_submodules(filter, true);
            res.insert(more.begin(), more.end());
        }
    }
    return res;
}

bool module::contains_module(const std::shared_ptr<module>& other, bool recursive) const
{
    if (other == nullptr)
    {
        return false;
    }
    for (const auto& sm : m_submodules_set)
    {
        if (sm == other)
        {
            return true;
        }
        else if (recursive && sm->contains_module(other, true))
        {
            return true;
        }
    }

    return false;
}

std::shared_ptr<netlist> module::get_netlist() const
{
    return m_internal_manager->m_netlist->get_shared();
}

bool module::assign_gate(const std::shared_ptr<gate>& gate)
{
    return m_internal_manager->module_assign_gate(shared_from_this(), gate);
}

bool module::remove_gate(const std::shared_ptr<gate>& gate)
{
    return m_internal_manager->module_remove_gate(shared_from_this(), gate);
}

bool module::contains_gate(const std::shared_ptr<gate>& gate, bool recursive) const
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

std::set<std::shared_ptr<gate>> module::get_gates(const std::function<bool(const std::shared_ptr<gate>&)>& filter, bool recursive) const
{
    std::set<std::shared_ptr<gate>> res;
    if (!filter)
    {
        res = m_gates_set;
    }
    else
    {
        for (const auto& g : m_gates_set)
        {
            if (!filter(g))
            {
                continue;
            }
            res.insert(g);
        }
    }

    if (recursive)
    {
        for (const auto& sm : m_submodules_set)
        {
            auto more = sm->get_gates(filter, true);
            res.insert(more.begin(), more.end());
        }
    }

    return res;
}

std::set<std::shared_ptr<net>> module::get_input_nets() const
{
    std::unordered_set<u32> seen;
    std::set<std::shared_ptr<net>> res;
    auto gates = get_gates(nullptr, true);
    for (const auto& gate : gates)
    {
        for (const auto& net : gate->get_fan_in_nets())
        {
            if (seen.find(net->get_id()) != seen.end())
            {
                continue;
            }
            seen.insert(net->get_id());
            if (m_internal_manager->m_netlist->is_global_input_net(net))
            {
                res.insert(net);
                continue;
            }
            auto sources = net->get_sources();
            if (std::any_of(sources.begin(), sources.end(), [&gates](endpoint src) { return gates.find(src.get_gate()) == gates.end(); }))
            {
                res.insert(net);
            }
        }
    }
    return res;
}

std::set<std::shared_ptr<net>> module::get_output_nets() const
{
    std::unordered_set<u32> seen;
    std::set<std::shared_ptr<net>> res;
    auto gates = get_gates(nullptr, true);
    for (const auto& gate : gates)
    {
        for (const auto& net : gate->get_fan_out_nets())
        {
            if (seen.find(net->get_id()) != seen.end())
            {
                continue;
            }
            seen.insert(net->get_id());
            if (m_internal_manager->m_netlist->is_global_output_net(net))
            {
                res.insert(net);
                continue;
            }
            auto destinations = net->get_destinations();
            if (std::any_of(destinations.begin(), destinations.end(), [&gates](endpoint dst) { return gates.find(dst.get_gate()) == gates.end(); }))
            {
                res.insert(net);
            }
        }
    }
    return res;
}

std::set<std::shared_ptr<net>> module::get_internal_nets() const
{
    std::unordered_set<u32> seen;
    std::set<std::shared_ptr<net>> res;
    auto gates = get_gates(nullptr, true);
    for (const auto& gate : gates)
    {
        for (const auto& net : gate->get_fan_out_nets())
        {
            if (seen.find(net->get_id()) != seen.end())
            {
                continue;
            }
            seen.insert(net->get_id());
            auto destinations = net->get_destinations();
            if (std::any_of(destinations.begin(), destinations.end(), [&gates](endpoint dst) { return gates.find(dst.get_gate()) != gates.end(); }))
            {
                res.insert(net);
            }
        }
    }
    return res;
}

void module::set_input_port_name(const std::shared_ptr<net>& input_net, const std::string& port_name)
{
    auto input_nets = get_input_nets();

    if (auto it = input_nets.find(input_net); it == input_nets.end())
    {
        log_warning(
            "module", "net '{}' with id {} is not an input net of module '{}' with id {}, ignoring port assignment", input_net->get_name(), input_net->get_id(), this->get_name(), this->get_id());
        return;
    }

    m_named_input_nets.insert(input_net);
    m_input_net_to_port_name.insert_or_assign(input_net, port_name);

    module_event_handler::notify(module_event_handler::event::input_port_name_changed, shared_from_this(), input_net->get_id());
}

void module::set_output_port_name(const std::shared_ptr<net>& output_net, const std::string& port_name)
{
    auto output_nets = get_output_nets();

    if (auto it = output_nets.find(output_net); it == output_nets.end())
    {
        log_warning(
            "module", "net '{}' with id {} is not an output net of module '{}' with id {}, ignoring port assignment", output_net->get_name(), output_net->get_id(), this->get_name(), this->get_id());
        return;
    }

    m_named_input_nets.insert(output_net);
    m_output_net_to_port_name.insert_or_assign(output_net, port_name);

    module_event_handler::notify(module_event_handler::event::output_port_name_changed, shared_from_this(), output_net->get_id());
}

std::string module::get_input_port_name(const std::shared_ptr<net>& input_net)
{
    std::string port_name;
    auto input_nets = get_input_nets();

    if (auto it = input_nets.find(input_net); it == input_nets.end())
    {
        log_warning("module", "net '{}' with id {} is not an input net of module '{}' with id {}.", input_net->get_name(), input_net->get_id(), this->get_name(), this->get_id());
        return "";
    }

    if (auto it = m_input_net_to_port_name.find(input_net); it != m_input_net_to_port_name.end())
    {
        port_name = it->second;
    }
    else
    {
        port_name = "I(" + std::to_string(m_next_input_port_id++) + ")";
        m_named_input_nets.insert(input_net);
        m_input_net_to_port_name.emplace(input_net, port_name);
    }

    return port_name;
}

std::string module::get_output_port_name(const std::shared_ptr<net>& output_net)
{
    std::string port_name;
    auto output_nets = get_output_nets();

    if (auto it = output_nets.find(output_net); it == output_nets.end())
    {
        log_warning("module", "net '{}' with id {} is not an output net of module '{}' with id {}.", output_net->get_name(), output_net->get_id(), this->get_name(), this->get_id());
        return "";
    }

    if (auto it = m_output_net_to_port_name.find(output_net); it != m_output_net_to_port_name.end())
    {
        port_name = it->second;
    }
    else
    {
        port_name = "O(" + std::to_string(m_next_input_port_id++) + ")";
        m_named_output_nets.insert(output_net);
        m_output_net_to_port_name.emplace(output_net, port_name);
    }

    return port_name;
}

const std::map<std::shared_ptr<net>, std::string>& module::get_input_port_names()
{
    auto input_nets = get_input_nets();
    std::set<std::shared_ptr<net>> diff;

    // find nets that are still in the port map but no longer an input net
    std::set_difference(m_named_input_nets.begin(), m_named_input_nets.end(), input_nets.begin(), input_nets.end(), std::inserter(diff, diff.begin()));
    for (const auto& net : diff)
    {
        m_named_input_nets.erase(net);
        m_input_net_to_port_name.erase(net);
    }

    diff.clear();

    // find nets that are input nets but have not yet been assigned a port name
    std::set_difference(input_nets.begin(), input_nets.end(), m_named_input_nets.begin(), m_named_input_nets.end(), std::inserter(diff, diff.begin()));
    for (const auto& net : diff)
    {
        m_named_input_nets.insert(net);
        m_input_net_to_port_name.emplace(net, "I(" + std::to_string(m_next_input_port_id++) + ")");
    }

    return m_input_net_to_port_name;
}

const std::map<std::shared_ptr<net>, std::string>& module::get_output_port_names()
{
    auto output_nets = get_output_nets();
    std::set<std::shared_ptr<net>> diff;

    // find nets that are still in the port map but no longer an input net
    std::set_difference(m_named_output_nets.begin(), m_named_output_nets.end(), output_nets.begin(), output_nets.end(), std::inserter(diff, diff.begin()));
    for (const auto& net : diff)
    {
        m_named_output_nets.erase(net);
        m_output_net_to_port_name.erase(net);
    }

    diff.clear();

    // find nets that are input nets but have not yet been assigned a port name
    std::set_difference(output_nets.begin(), output_nets.end(), m_named_output_nets.begin(), m_named_output_nets.end(), std::inserter(diff, diff.begin()));
    for (const auto& net : diff)
    {
        m_named_output_nets.insert(net);
        m_output_net_to_port_name.emplace(net, "O(" + std::to_string(m_next_input_port_id++) + ")");
    }

    return m_output_net_to_port_name;
}
