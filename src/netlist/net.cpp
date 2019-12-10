#include "netlist/net.h"

#include "netlist/gate.h"
#include "netlist/netlist.h"
#include "netlist/netlist_internal_manager.h"

#include "netlist/event_system/net_event_handler.h"

#include "core/log.h"

#include <assert.h>
#include <memory>

net::net(netlist_internal_manager* internal_manager, const u32 id, const std::string& name)
{
    assert(internal_manager != nullptr);
    m_internal_manager = internal_manager;
    m_id               = id;
    m_name             = name;
    m_src              = {nullptr, ""};
}

u32 net::get_id() const
{
    return m_id;
}

std::shared_ptr<netlist> net::get_netlist() const
{
    return m_internal_manager->m_netlist->get_shared();
}

std::string net::get_name() const
{
    return m_name;
}

void net::set_name(const std::string& name)
{
    if (core_utils::trim(name).empty())
    {
        log_error("netlist.internal", "net::set_name: empty name is not allowed");
        return;
    }
    if (name != m_name)
    {
        log_info("netlist.internal", "changed name for net (id = {}) from '{}' to '{}'.", m_id, m_name, name);

        m_name = name;

        net_event_handler::notify(net_event_handler::event::name_changed, shared_from_this());
    }
}

bool net::set_src(const std::shared_ptr<gate>& gate, const std::string& pin_type)
{
    return set_src({gate, pin_type});
}

bool net::set_src(const endpoint& src)
{
    return m_internal_manager->net_set_src(shared_from_this(), src);
}

bool net::remove_src()
{
    return m_internal_manager->net_remove_src(shared_from_this());
}

endpoint net::get_src() const
{
    return m_src;
}

bool net::add_dst(const std::shared_ptr<gate>& gate, const std::string& pin_type)
{
    return add_dst({gate, pin_type});
}

bool net::add_dst(const endpoint& dst)
{
    return m_internal_manager->net_add_dst(shared_from_this(), dst);
}

bool net::remove_dst(const std::shared_ptr<gate>& gate, const std::string& pin_type)
{
    return remove_dst({gate, pin_type});
}

bool net::remove_dst(const endpoint& dst)
{
    return m_internal_manager->net_remove_dst(shared_from_this(), dst);
}

bool net::is_a_dst(const std::shared_ptr<gate>& gate) const
{
    if (gate == nullptr)
    {
        log_error("netlist", "parameter 'gate' is nullptr.");
        return false;
    }
    if (!get_netlist()->is_gate_in_netlist(gate))
    {
        log_error("netlist", "gate '{}' does not belong to netlist.", gate->get_name());
        return false;
    }

    for (const auto& dst : m_dsts)
    {
        if (dst.gate == gate)
        {
            return true;
        }
    }
    return false;
}

bool net::is_a_dst(const endpoint& ep) const
{
    if (ep.gate == nullptr)
    {
        log_error("netlist", "parameter 'gate' is nullptr.");
        return false;
    }
    if (!get_netlist()->is_gate_in_netlist(ep.gate))
    {
        log_error("netlist", "gate '{}' does not belong to netlist.", ep.gate->get_name());
        return false;
    }

    return std::find(m_dsts.begin(), m_dsts.end(), ep) != m_dsts.end();
}

u32 net::get_num_of_dsts() const
{
    return (u32)m_dsts.size();
}

std::vector<endpoint> net::get_dsts(const std::function<bool(const endpoint& ep)>& filter) const
{
    if (!filter)
    {
        return m_dsts;
    }

    std::vector<endpoint> dsts;
    for (const auto& dst : m_dsts)
    {
        if (!filter(dst))
        {
            continue;
        }
        dsts.push_back(dst);
    }
    return dsts;
}


bool net::is_unrouted() const
{
    return ((m_src.gate == nullptr) || (this->get_num_of_dsts() == 0));
}

bool net::mark_global_input_net()
{
    return m_internal_manager->m_netlist->mark_global_input_net(shared_from_this());
}

bool net::mark_global_output_net()
{
    return m_internal_manager->m_netlist->mark_global_output_net(shared_from_this());
}

bool net::unmark_global_input_net()
{
    return m_internal_manager->m_netlist->unmark_global_input_net(shared_from_this());
}

bool net::unmark_global_output_net()
{
    return m_internal_manager->m_netlist->unmark_global_output_net(shared_from_this());
}

bool net::is_global_input_net() const
{
    return m_internal_manager->m_netlist->is_global_input_net(const_cast<net*>(this)->shared_from_this());
}

bool net::is_global_output_net() const
{
    return m_internal_manager->m_netlist->is_global_output_net(const_cast<net*>(this)->shared_from_this());
}
