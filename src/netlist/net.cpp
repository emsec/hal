#include "netlist/net.h"

#include "core/log.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/gate.h"
#include "netlist/netlist.h"
#include "netlist/netlist_internal_manager.h"

#include <assert.h>
#include <memory>

namespace hal
{
    Net::Net(NetlistInternalManager* internal_manager, const u32 id, const std::string& name)
    {
        assert(internal_manager != nullptr);
        m_internal_manager = internal_manager;
        m_id               = id;
        m_name             = name;
    }

    u32 Net::get_id() const
    {
        return m_id;
    }

    Netlist* Net::get_netlist() const
    {
        return m_internal_manager->m_netlist->get_shared();
    }

    std::string Net::get_name() const
    {
        return m_name;
    }

    void Net::set_name(const std::string& name)
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

            net_event_handler::notify(net_event_handler::event::name_changed, this);
        }
    }

    bool Net::add_source(Gate* gate, const std::string& pin)
    {
        return add_source(Endpoint(gate, pin, false));
    }

    bool Net::add_source(const Endpoint& ep)
    {
        if (!ep.is_source_pin())
        {
            log_error("netlist", "net::add_source: tried to use a destination-endpoint as a source-endpoint");
            return false;
        }
        return m_internal_manager->net_add_source(this, ep);
    }

    bool Net::remove_source(Gate* gate, const std::string& pin)
    {
        return remove_source(Endpoint(gate, pin, false));
    }

    bool Net::remove_source(const Endpoint& ep)
    {
        if (!ep.is_source_pin())
        {
            log_error("netlist", "net::remove_source: tried to use a destination-endpoint as a source-endpoint");
            return false;
        }
        return m_internal_manager->net_remove_source(this, ep);
    }

    bool Net::is_a_source(Gate* gate, const std::string& pin) const
    {
        return is_a_source(Endpoint(gate, pin, false));
    }

    bool Net::is_a_source(const Endpoint& ep) const
    {
        if (!ep.is_source_pin())
        {
            log_error("netlist", "net::is_a_source: tried to use a destination-endpoint as a source-endpoint");
            return false;
        }

        return std::find(m_sources.begin(), m_sources.end(), ep) != m_sources.end();
    }

    u32 Net::get_num_of_sources() const
    {
        return (u32)m_sources.size();
    }

    std::vector<Endpoint> Net::get_sources(const std::function<bool(const Endpoint& ep)>& filter) const
    {
        if (!filter)
        {
            return m_sources;
        }

        std::vector<Endpoint> srcs;
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

    Endpoint Net::get_source() const
    {
        if (m_sources.empty())
        {
            return Endpoint(nullptr, "", false);
        }
        if (m_sources.size() > 1)
        {
            log_warning("netlist", "queried only the first source of multi driven net '{}' (id {})", m_name, m_id);
        }
        return m_sources.at(0);
    }

    bool Net::add_destination(Gate* gate, const std::string& pin)
    {
        return add_destination(Endpoint(gate, pin, true));
    }

    bool Net::add_destination(const Endpoint& ep)
    {
        if (!ep.is_destination_pin())
        {
            log_error("netlist", "net::add_destination: tried to use a source-endpoint as a destination-endpoint");
            return false;
        }
        return m_internal_manager->net_add_destination(this, ep);
    }

    bool Net::remove_destination(Gate* gate, const std::string& pin)
    {
        return remove_destination(Endpoint(gate, pin, true));
    }

    bool Net::remove_destination(const Endpoint& ep)
    {
        if (!ep.is_destination_pin())
        {
            log_error("netlist", "net::remove_destination: tried to use a source-endpoint as a destination-endpoint");
            return false;
        }
        return m_internal_manager->net_remove_destination(this, ep);
    }

    bool Net::is_a_destination(Gate* gate, const std::string& pin) const
    {
        return is_a_destination(Endpoint(gate, pin, true));
    }

    bool Net::is_a_destination(const Endpoint& ep) const
    {
        if (!ep.is_destination_pin())
        {
            log_error("netlist", "net::is_a_destination: tried to use a source-endpoint as a destination-endpoint");
            return false;
        }

        return std::find(m_destinations.begin(), m_destinations.end(), ep) != m_destinations.end();
    }

    u32 Net::get_num_of_destinations() const
    {
        return (u32)m_destinations.size();
    }

    std::vector<Endpoint> Net::get_destinations(const std::function<bool(const Endpoint& ep)>& filter) const
    {
        if (!filter)
        {
            return m_destinations;
        }

        std::vector<Endpoint> dsts;
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

    bool Net::is_unrouted() const
    {
        return ((this->get_num_of_sources() == 0) || (this->get_num_of_destinations() == 0));
    }

    bool Net::mark_global_input_net()
    {
        return m_internal_manager->m_netlist->mark_global_input_net(this);
    }

    bool Net::mark_global_output_net()
    {
        return m_internal_manager->m_netlist->mark_global_output_net(this);
    }

    bool Net::unmark_global_input_net()
    {
        return m_internal_manager->m_netlist->unmark_global_input_net(this);
    }

    bool Net::unmark_global_output_net()
    {
        return m_internal_manager->m_netlist->unmark_global_output_net(this);
    }

    bool Net::is_global_input_net()
    {
        return m_internal_manager->m_netlist->is_global_input_net(this);
    }

    bool Net::is_global_output_net()
    {
        return m_internal_manager->m_netlist->is_global_output_net(this);
    }
}    // namespace hal
