#include "netlist/netlist_internal_manager.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/gate_library/gate_type.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include "netlist/event_system/net_event_handler.h"

netlist_internal_manager::netlist_internal_manager(netlist* nl) : m_netlist(nl)
{
    assert(nl != nullptr);
}

//######################################################################
//###                      gates                                     ###
//######################################################################

std::shared_ptr<gate> netlist_internal_manager::create_gate(const u32 id, std::shared_ptr<const gate_type> gt, const std::string& name)
{
    if (id == 0)
    {
        log_error("netlist.internal", "netlist::create_gate: id 0 represents 'invalid ID'.");
        return nullptr;
    }
    if (m_netlist->m_used_gate_ids.find(id) != m_netlist->m_used_gate_ids.end())
    {
        log_error("netlist.internal", "netlist::create_gate: gate id {:08x} is already taken.", id);
        return nullptr;
    }
    if (this->is_gate_type_invalid(gt))
    {
        log_error("netlist.internal", "netlist::create_gate: gate type '{}' is invalid.", gt->get_name());
        return nullptr;
    }
    if (core_utils::trim(name).empty())
    {
        log_error("netlist.internal", "netlist::create_gate: empty name is not allowed");
        return nullptr;
    }

    auto new_gate = std::shared_ptr<gate>(new gate(m_netlist->get_shared(), id, gt, name));

    auto free_id_it = m_netlist->m_free_gate_ids.find(id);
    if (free_id_it != m_netlist->m_free_gate_ids.end())
    {
        m_netlist->m_free_gate_ids.erase(free_id_it);
    }

    m_netlist->m_used_gate_ids.insert(id);

    // add gate to top module
    new_gate->m_module                       = m_netlist->m_top_module;
    m_netlist->m_top_module->m_gates_map[id] = new_gate;
    m_netlist->m_top_module->m_gates_set.insert(new_gate);

    // notify
    module_event_handler::notify(module_event_handler::event::gate_assigned, m_netlist->m_top_module, id);
    gate_event_handler::notify(gate_event_handler::event::created, new_gate);

    return new_gate;
}

bool netlist_internal_manager::delete_gate(std::shared_ptr<gate> gate)
{
    if (!m_netlist->is_gate_in_netlist(gate))
    {
        return false;
    }

    for (const auto& fan_in_net : gate->get_fan_in_nets())
    {
        if (fan_in_net->is_a_dst({gate, DONT_CARE}) && !this->net_remove_dst(fan_in_net, {gate, DONT_CARE}))
        {
            return false;
        }
    }
    for (const auto& fan_out_net : gate->get_fan_out_nets())
    {
        if (fan_out_net->m_src.gate != nullptr && !this->net_remove_src(fan_out_net))
        {
            return false;
        }
    }

    // check global_gnd and global_vcc gates
    m_netlist->unmark_global_gnd_gate(gate);
    m_netlist->unmark_global_vcc_gate(gate);

    // remove gate from modules
    gate->m_module->m_gates_map.erase(gate->m_module->m_gates_map.find(gate->get_id()));
    gate->m_module->m_gates_set.erase(gate);

    // free ids
    m_netlist->m_free_gate_ids.insert(gate->get_id());
    m_netlist->m_used_gate_ids.erase(gate->get_id());

    module_event_handler::notify(module_event_handler::event::gate_removed, gate->m_module, gate->get_id());
    gate_event_handler::notify(gate_event_handler::event::removed, gate);

    return true;
}

bool netlist_internal_manager::is_gate_type_invalid(std::shared_ptr<const gate_type> gt) const
{
    auto gate_types = m_netlist->m_gate_library->get_gate_types();
    return std::find_if(gate_types.begin(), gate_types.end(), [&](std::shared_ptr<gate_type> const& it) { return *it == *gt; }) == gate_types.end();
}

//######################################################################
//###                       nets                                     ###
//######################################################################

std::shared_ptr<net> netlist_internal_manager::create_net(const u32 id, const std::string& name)
{
    if (id == 0)
    {
        log_error("netlist.internal", "netlist::create_net: id 0 represents 'invalid ID'.");
        return nullptr;
    }
    if (m_netlist->m_used_net_ids.find(id) != m_netlist->m_used_net_ids.end())
    {
        log_error("netlist.internal", "netlist::create_net: net id {:08x} is already taken.", id);
        return nullptr;
    }
    if (core_utils::trim(name).empty())
    {
        log_error("netlist.internal", "netlist::create_net: empty name is not allowed");
        return nullptr;
    }

    auto new_net = std::shared_ptr<net>(new net(this, id, name));

    auto free_id_it = m_netlist->m_free_net_ids.find(id);
    if (free_id_it != m_netlist->m_free_net_ids.end())
    {
        m_netlist->m_free_net_ids.erase(free_id_it);
    }

    m_netlist->m_used_net_ids.insert(id);

    // add net to netlist
    m_netlist->m_nets_map[id] = new_net;
    m_netlist->m_nets_set.insert(new_net);

    // notify
    net_event_handler::notify(net_event_handler::event::created, new_net);

    return new_net;
}

bool netlist_internal_manager::delete_net(std::shared_ptr<net> net)
{
    if (!m_netlist->is_net_in_netlist(net))
    {
        return false;
    }

    auto dsts = net->m_dsts;
    for (const auto& dst : dsts)
    {
        if (net->is_a_dst(dst) && !this->net_remove_dst(net, dst))
        {
            return false;
        }
    }

    if (net->m_src.gate != nullptr && !this->net_remove_src(net))
    {
        return false;
    }

    // check global_input, global_output and global_inout gates
    m_netlist->unmark_global_input_net(net);
    m_netlist->unmark_global_output_net(net);
    m_netlist->unmark_global_inout_net(net);

    // remove net from netlist
    m_netlist->m_nets_map.erase(m_netlist->m_nets_map.find(net->get_id()));
    m_netlist->m_nets_set.erase(net);

    m_netlist->m_free_net_ids.insert(net->get_id());
    m_netlist->m_used_net_ids.erase(net->get_id());

    net_event_handler::notify(net_event_handler::event::removed, net);

    return true;
}

bool netlist_internal_manager::net_set_src(std::shared_ptr<net> const net, endpoint src)
{
    if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(src.gate))
    {
        return false;
    }

    // check whether pin is valid for this gate
    auto output_pin_types = src.gate->get_type()->get_output_pins();
    if ((std::find(output_pin_types.begin(), output_pin_types.end(), src.pin_type) == output_pin_types.end()))
    {
        log_error("netlist.internal", "net::set_src: src gate ('{}, type = {}) has no output type '{}'.", src.gate->get_name(), src.gate->get_type()->get_name(), src.pin_type);
        return false;
    }

    // check whether src already belongs to other net
    for (const auto& out_net : src.gate->get_fan_out_nets())
    {
        if ((out_net->get_src() == src) && (net != out_net))
        {
            log_error("netlist.internal",
                      "net::set_src: src gate ('{}', {}) has already associated net '{}'. Cannot assign {} as new src.",
                      src.gate->get_name(),
                      src.pin_type,
                      out_net->get_name(),
                      net->get_name());
            return false;
        }
    }

    // check whether net has already assigned src (if so remove it first)
    if (net->m_src.gate != nullptr && !this->net_remove_src(net))
    {
        return false;
    }

    net->m_src                         = src;
    src.gate->m_out_nets[src.pin_type] = net;

    net_event_handler::notify(net_event_handler::event::src_changed, net);

    return true;
}

bool netlist_internal_manager::net_remove_src(std::shared_ptr<net> const net)
{
    if (!m_netlist->is_net_in_netlist(net))
    {
        return false;
    }

    if (net->m_src.gate == nullptr)
    {
        log_error("netlist.internal", "net::remove_src: src of net '{}' is already empty.", net->get_name());
        return false;
    }

    auto old_src = net->m_src;

    net->m_src.gate->m_out_nets.erase(net->m_src.pin_type);
    net->m_src = {nullptr, ""};

    net_event_handler::notify(net_event_handler::event::src_changed, net);

    return true;
}

bool netlist_internal_manager::net_add_dst(std::shared_ptr<net> const net, endpoint dst)
{
    if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(dst.gate))
    {
        return false;
    }

    if (net->is_a_dst(dst.gate, dst.pin_type))
    {
        log_error("netlist.internal", "net::add_dst: dst gate ('{}',  type = {}) is already added to net '{}'.", dst.gate->get_name(), dst.gate->get_type()->get_name(), net->get_name());
        return false;
    }

    // check whether pin id is valid for this gate
    auto input_pin_types = dst.gate->get_type()->get_input_pins();

    if ((std::find(input_pin_types.begin(), input_pin_types.end(), dst.pin_type) == input_pin_types.end()))
    {
        log_error("netlist.internal", "net::add_dst: dst gate ('{}',  type = {}) has no input type '{}'.", dst.gate->get_name(), dst.gate->get_type()->get_name(), dst.pin_type);
        return false;
    }

    // check whether dst has already assigned src
    if (dst.gate->get_fan_in_net(dst.pin_type) != nullptr)
    {
        log_error("netlist.internal",
                  "net::add_dst: dst gate ('{}', type = {}) has already an assigned net '{}' for pin '{}' (new_net: {}).",
                  dst.gate->get_name(),
                  dst.gate->get_type()->get_name(),
                  dst.gate->get_fan_in_net(dst.pin_type)->get_name(),
                  dst.pin_type,
                  net->get_name());
        return false;
    }

    net->m_dsts.push_back(dst);
    dst.gate->m_in_nets[dst.pin_type] = net;

    net_event_handler::notify(net_event_handler::event::dst_added, net, dst.gate->get_id());

    return true;
}

bool netlist_internal_manager::net_remove_dst(std::shared_ptr<net> const net, endpoint dst)
{
    if (!m_netlist->is_net_in_netlist(net) || !m_netlist->is_gate_in_netlist(dst.gate) || !net->is_a_dst(dst.gate, dst.pin_type))
    {
        return false;
    }

    for (auto it = net->m_dsts.begin(); it != net->m_dsts.end();)
    {
        auto net_dst = *it;
        if ((net_dst.gate == dst.gate) && ((dst.pin_type == DONT_CARE) || (dst.pin_type == net_dst.pin_type)))
        {
            net_dst.gate->m_in_nets.erase(net_dst.pin_type);
            it = net->m_dsts.erase(it);
        }
        else
        {
            ++it;
        }
    }

    net_event_handler::notify(net_event_handler::event::dst_removed, net, dst.gate->get_id());

    return true;
}

//######################################################################
//###                       modules                               ###
//######################################################################

std::shared_ptr<module> netlist_internal_manager::create_module(const u32 id, std::shared_ptr<module> parent, const std::string& name)
{
    if (id == 0)
    {
        log_error("netlist.internal", "netlist::create_module: id 0 represents 'invalid ID'.");
        return nullptr;
    }
    if (m_netlist->m_used_module_ids.find(id) != m_netlist->m_used_module_ids.end())
    {
        log_error("netlist.internal", "netlist::create_module: module id {:08x} is already taken.", id);
        return nullptr;
    }
    if (core_utils::trim(name).empty())
    {
        log_error("netlist.internal", "netlist::create_module: empty name is not allowed");
        return nullptr;
    }
    if (parent == nullptr && m_netlist->m_top_module != nullptr)
    {
        log_error("netlist.internal", "netlist::create_module: parent must not be nullptr");
        return nullptr;
    }
    if (parent != nullptr && m_netlist->get_shared() != parent->get_netlist())
    {
        log_error("netlist.internal", "netlist::create_module: parent must belong to current netlist");
        return nullptr;
    }

    auto m = std::shared_ptr<module>(new module(id, parent, name, this));

    auto free_id_it = m_netlist->m_free_module_ids.find(id);
    if (free_id_it != m_netlist->m_free_module_ids.end())
    {
        m_netlist->m_free_module_ids.erase(free_id_it);
    }

    m_netlist->m_used_module_ids.insert(id);

    m_netlist->m_modules[id] = m;

    if (parent != nullptr)
    {
        parent->m_submodules_map[id] = m;
        parent->m_submodules_set.insert(m);
    }

    module_event_handler::notify(module_event_handler::event::created, m);
    module_event_handler::notify(module_event_handler::event::submodule_added, parent, id);

    return m;
}

bool netlist_internal_manager::delete_module(std::shared_ptr<module> to_remove)
{
    if (!m_netlist->is_module_in_netlist(to_remove))
    {
        return false;
    }

    if (to_remove == m_netlist->m_top_module)
    {
        return false;
    }

    // at this point parent is guaranteed to be not null

    // move gates and nets to parent, work on a copy since assign_gate will modify m_gates_set
    auto gates_copy = to_remove->m_gates_set;
    for (const auto& gate : gates_copy)
    {
        to_remove->m_parent->assign_gate(gate);
    }

    // move all submodules to parent
    for (const auto& sm : to_remove->m_submodules_set)
    {
        to_remove->m_parent->m_submodules_map[sm->get_id()] = sm;
        to_remove->m_parent->m_submodules_set.insert(sm);

        sm->m_parent = to_remove->m_parent;

        module_event_handler::notify(module_event_handler::event::submodule_added, to_remove->m_parent, sm->get_id());
        module_event_handler::notify(module_event_handler::event::parent_changed, sm, 0);
    }

    // remove module from parent
    to_remove->m_parent->m_submodules_map.erase(to_remove->get_id());
    to_remove->m_parent->m_submodules_set.erase(to_remove);
    module_event_handler::notify(module_event_handler::event::submodule_removed, to_remove->m_parent, to_remove->get_id());

    m_netlist->m_modules.erase(to_remove->get_id());

    m_netlist->m_free_module_ids.insert(to_remove->get_id());
    m_netlist->m_used_module_ids.erase(to_remove->get_id());

    module_event_handler::notify(module_event_handler::event::removed, to_remove);
    return true;
}

bool netlist_internal_manager::module_assign_gate(std::shared_ptr<module> m, std::shared_ptr<gate> g)
{
    if (g == nullptr)
    {
        return false;
    }
    if (g->m_module == m)
    {
        return false;
    }
    auto prev_module = g->m_module;

    prev_module->m_gates_map.erase(prev_module->m_gates_map.find(g->get_id()));
    prev_module->m_gates_set.erase(g);

    m->m_gates_map[g->get_id()] = g;
    m->m_gates_set.insert(g);

    g->m_module = m;

    module_event_handler::notify(module_event_handler::event::gate_removed, prev_module, g->get_id());
    module_event_handler::notify(module_event_handler::event::gate_assigned, m, g->get_id());
    return true;
}

bool netlist_internal_manager::module_remove_gate(std::shared_ptr<module> m, std::shared_ptr<gate> g)
{
    if (g == nullptr)
    {
        return false;
    }

    if (m == m_netlist->m_top_module)
    {
        log_error("module", "cannot remove gates from top module.", g->get_name(), g->get_id(), m->get_name(), m->get_id());
        return false;
    }

    auto it = m->m_gates_map.find(g->get_id());

    if (it == m->m_gates_map.end())
    {
        log_error("module", "gate '{}' (id {}) is not stored in module '{}' (id {}).", g->get_name(), g->get_id(), m->get_name(), m->get_id());
        return false;
    }

    m->m_gates_map.erase(it);
    m->m_gates_set.erase(g);

    m_netlist->m_top_module->m_gates_map[g->get_id()] = g;
    m_netlist->m_top_module->m_gates_set.insert(g);
    g->m_module = m_netlist->m_top_module;

    module_event_handler::notify(module_event_handler::event::gate_removed, m, g->get_id());
    module_event_handler::notify(module_event_handler::event::gate_assigned, m_netlist->m_top_module, g->get_id());

    return true;
}
