#include "hal_core/netlist/net.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_system/net_event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/log.h"

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
        return m_internal_manager->m_netlist;
    }

    std::string Net::get_name() const
    {
        return m_name;
    }

    void Net::set_name(const std::string& name)
    {
        if (utils::trim(name).empty())
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

    Grouping* Net::get_grouping() const
    {
        return m_grouping;
    }

    Endpoint* Net::add_source(Gate* gate, const std::string& pin)
    {
        return m_internal_manager->net_add_source(this, gate, pin);
    }

    bool Net::remove_source(Gate* gate, const std::string& pin)
    {
        if (auto it = std::find_if(m_sources_raw.begin(), m_sources_raw.end(), [gate, &pin](auto ep) { return ep->get_gate() == gate && ep->get_pin() == pin; }); it != m_sources_raw.end())
        {
            return m_internal_manager->net_remove_source(this, *it);
        }
        return false;
    }

    bool Net::remove_source(Endpoint* ep)
    {
        return m_internal_manager->net_remove_source(this, ep);
    }

    bool Net::is_a_source(Gate* gate, const std::string& pin) const
    {
        return std::find_if(m_sources_raw.begin(), m_sources_raw.end(), [gate, &pin](auto ep) { return ep->get_gate() == gate && ep->get_pin() == pin; }) != m_sources_raw.end();
    }

    bool Net::is_a_source(Endpoint* ep) const
    {
        if (ep == nullptr)
        {
            return false;
        }
        if (!ep->is_source_pin())
        {
            log_error("netlist", "net::is_a_source: tried to use a destination-endpoint as a source-endpoint");
            return false;
        }

        return std::find(m_sources_raw.begin(), m_sources_raw.end(), ep) != m_sources_raw.end();
    }

    u32 Net::get_num_of_sources() const
    {
        return (u32)m_sources_raw.size();
    }

    std::vector<Endpoint*> Net::get_sources(const std::function<bool(Endpoint* ep)>& filter) const
    {
        if (!filter)
        {
            return m_sources_raw;
        }

        std::vector<Endpoint*> srcs;
        for (auto src : m_sources_raw)
        {
            if (!filter(src))
            {
                continue;
            }
            srcs.push_back(src);
        }
        return srcs;
    }

    Endpoint* Net::get_source() const
    {
        // log_warning("netlist", "Net::get_source() is deprecated");
        if (m_sources_raw.empty())
        {
            return nullptr;
        }
        if (m_sources_raw.size() > 1)
        {
            log_warning("netlist", "queried only the first source of multi driven net '{}' (id {})", m_name, m_id);
        }
        return m_sources_raw.at(0);
    }

    Endpoint* Net::add_destination(Gate* gate, const std::string& pin)
    {
        return m_internal_manager->net_add_destination(this, gate, pin);
    }

    bool Net::remove_destination(Gate* gate, const std::string& pin)
    {
        if (auto it = std::find_if(m_destinations_raw.begin(), m_destinations_raw.end(), [gate, &pin](auto ep) { return ep->get_gate() == gate && ep->get_pin() == pin; });
            it != m_destinations_raw.end())
        {
            return m_internal_manager->net_remove_destination(this, *it);
        }
        return false;
    }

    bool Net::remove_destination(Endpoint* ep)
    {
        return m_internal_manager->net_remove_destination(this, ep);
    }

    bool Net::is_a_destination(Gate* gate, const std::string& pin) const
    {
        return std::find_if(m_destinations_raw.begin(), m_destinations_raw.end(), [gate, &pin](auto ep) { return ep->get_gate() == gate && ep->get_pin() == pin; }) != m_destinations_raw.end();
    }

    bool Net::is_a_destination(Endpoint* ep) const
    {
        if (ep == nullptr)
        {
            return false;
        }
        if (!ep->is_destination_pin())
        {
            log_error("netlist", "net::is_a_destination: tried to use a source-endpoint as a destination-endpoint");
            return false;
        }

        return std::find(m_destinations_raw.begin(), m_destinations_raw.end(), ep) != m_destinations_raw.end();
    }

    u32 Net::get_num_of_destinations() const
    {
        return (u32)m_destinations_raw.size();
    }

    std::vector<Endpoint*> Net::get_destinations(const std::function<bool(Endpoint* ep)>& filter) const
    {
        if (!filter)
        {
            return m_destinations_raw;
        }

        std::vector<Endpoint*> dsts;
        for (auto dst : m_destinations_raw)
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
