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

bool net::add_source(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return add_source(endpoint(gate, pin, false));
}

bool net::add_source(const endpoint& ep)
{
    if (!ep.is_source_pin())
    {
        log_error("netlist", "net::add_source: tried to use a destination-endpoint as a source-endpoint");
        return false;
    }
    return m_internal_manager->net_add_source(shared_from_this(), ep);
}

bool net::remove_source(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return remove_source(endpoint(gate, pin, false));
}

bool net::remove_source(const endpoint& ep)
{
    if (!ep.is_source_pin())
    {
        log_error("netlist", "net::remove_source: tried to use a destination-endpoint as a source-endpoint");
        return false;
    }
    return m_internal_manager->net_remove_source(shared_from_this(), ep);
}

bool net::is_a_source(const std::shared_ptr<gate>& gate, const std::string& pin) const
{
    return is_a_source(endpoint(gate, pin, false));
}

bool net::is_a_source(const endpoint& ep) const
{
    if (!ep.is_source_pin())
    {
        log_error("netlist", "net::is_a_source: tried to use a destination-endpoint as a source-endpoint");
        return false;
    }

    return std::find(m_sources.begin(), m_sources.end(), ep) != m_sources.end();
}

u32 net::get_num_of_sources() const
{
    return (u32)m_sources.size();
}

std::vector<endpoint> net::get_sources(const std::function<bool(const endpoint& ep)>& filter) const
{
    if (!filter)
    {
        return m_sources;
    }

    std::vector<endpoint> srcs;
    for (const auto& src : m_sources)
    {
        if (!filter(src))
        {
            continue;
        }
        srcs.push_back(src);
    }
    return srcs;
}

endpoint net::get_source() const
{
    if (m_sources.empty())
    {
        return endpoint(nullptr, "", false);
    }
    if (m_sources.size() > 1)
    {
        log_warning("netlist", "queried only the first source of multi driven net '{}' (id {})", m_name, m_id);
    }
    return m_sources.at(0);
}

bool net::add_destination(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return add_destination(endpoint(gate, pin, true));
}

bool net::add_destination(const endpoint& ep)
{
    if (!ep.is_destination_pin())
    {
        log_error("netlist", "net::add_destination: tried to use a source-endpoint as a destination-endpoint");
        return false;
    }
    return m_internal_manager->net_add_destination(shared_from_this(), ep);
}

bool net::remove_destination(const std::shared_ptr<gate>& gate, const std::string& pin)
{
    return remove_destination(endpoint(gate, pin, true));
}

bool net::remove_destination(const endpoint& ep)
{
    if (!ep.is_destination_pin())
    {
        log_error("netlist", "net::remove_destination: tried to use a source-endpoint as a destination-endpoint");
        return false;
    }
    return m_internal_manager->net_remove_destination(shared_from_this(), ep);
}

bool net::is_a_destination(const std::shared_ptr<gate>& gate, const std::string& pin) const
{
    return is_a_destination(endpoint(gate, pin, true));
}

bool net::is_a_destination(const endpoint& ep) const
{
    if (!ep.is_destination_pin())
    {
        log_error("netlist", "net::is_a_destination: tried to use a source-endpoint as a destination-endpoint");
        return false;
    }

    return std::find(m_destinations.begin(), m_destinations.end(), ep) != m_destinations.end();
}

u32 net::get_num_of_destinations() const
{
    return (u32)m_destinations.size();
}

std::vector<endpoint> net::get_destinations(const std::function<bool(const endpoint& ep)>& filter) const
{
    if (!filter)
    {
        return m_destinations;
    }

    std::vector<endpoint> dsts;
    for (const auto& dst : m_destinations)
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
    return ((this->get_num_of_sources() == 0) || (this->get_num_of_destinations() == 0));
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
