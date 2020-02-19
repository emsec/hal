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

bool net::add_src(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return add_src(endpoint(gate, pin, false));
}

bool net::add_src(const endpoint& ep)
{
    if (!ep.is_src_pin())
    {
        log_error("netlist", "net::add_src: tried to use a destination-endpoint as a source-endpoint");
        return false;
    }
    return m_internal_manager->net_add_src(shared_from_this(), ep);
}

bool net::remove_src(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return remove_src(endpoint(gate, pin, false));
}

bool net::remove_src(const endpoint& ep)
{
    if (!ep.is_src_pin())
    {
        log_error("netlist", "net::remove_src: tried to use a destination-endpoint as a source-endpoint");
        return false;
    }
    return m_internal_manager->net_remove_src(shared_from_this(), ep);
}

bool net::is_a_src(const std::shared_ptr<gate>& gate, const std::string& pin) const
{
    return is_a_src(endpoint(gate, pin, false));
}

bool net::is_a_src(const endpoint& ep) const
{
    if (!ep.is_src_pin())
    {
        log_error("netlist", "net::is_a_src: tried to use a destination-endpoint as a source-endpoint");
        return false;
    }

    return std::find(m_srcs.begin(), m_srcs.end(), ep) != m_srcs.end();
}

u32 net::get_num_of_srcs() const
{
    return (u32)m_srcs.size();
}

std::vector<endpoint> net::get_srcs(const std::function<bool(const endpoint& ep)>& filter) const
{
    if (!filter)
    {
        return m_srcs;
    }

    std::vector<endpoint> srcs;
    for (const auto& src : m_srcs)
    {
        if (!filter(src))
        {
            continue;
        }
        srcs.push_back(src);
    }
    return srcs;
}

endpoint net::get_src() const
{
    if (m_srcs.empty())
    {
        return endpoint(nullptr, "", false);
    }
    if (m_srcs.size() > 1)
    {
        log_warning("netlist", "queried only the first source of multi driven net '{}' (id {})", m_name, m_id);
    }
    return m_srcs.at(0);
}

bool net::add_dst(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return add_dst(endpoint(gate, pin, true));
}

bool net::add_dst(const endpoint& ep)
{
    if (!ep.is_dst_pin())
    {
        log_error("netlist", "net::add_dst: tried to use a source-endpoint as a destination-endpoint");
        return false;
    }
    return m_internal_manager->net_add_dst(shared_from_this(), ep);
}

bool net::remove_dst(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return remove_dst(endpoint(gate, pin, true));
}

bool net::remove_dst(const endpoint& ep)
{
    if (!ep.is_dst_pin())
    {
        log_error("netlist", "net::remove_dst: tried to use a source-endpoint as a destination-endpoint");
        return false;
    }
    return m_internal_manager->net_remove_dst(shared_from_this(), ep);
}

bool net::is_a_dst(const std::shared_ptr<gate>& gate, const std::string& pin) const
{
    return is_a_dst(endpoint(gate, pin, true));
}

bool net::is_a_dst(const endpoint& ep) const
{
    if (!ep.is_dst_pin())
    {
        log_error("netlist", "net::is_a_dst: tried to use a source-endpoint as a destination-endpoint");
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
    return ((this->get_num_of_srcs() == 0) || (this->get_num_of_dsts() == 0));
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
