#include "hal_core/netlist/net.h"

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/log.h"

#include <assert.h>
#include <memory>

namespace hal
{
    Net::Net(NetlistInternalManager* internal_manager, EventHandler* event_handler, const u32 id, const std::string& name)
    {
        assert(internal_manager != nullptr);
        m_internal_manager = internal_manager;
        m_id               = id;
        m_name             = name;

        m_event_handler = event_handler;
    }

    bool Net::operator==(const Net& other) const
    {
        if (m_id != other.get_id() || m_name != other.get_name())
        {
            log_debug("net", "the nets with IDs {} and {} are not equal due to an unequal ID or name.", m_id, other.get_id());
            return false;
        }

        if (is_global_input_net() != other.is_global_input_net() || is_global_output_net() != other.is_global_output_net())
        {
            log_debug("net", "the nets with IDs {} and {} are not equal as one is a global input or output net and the other is not.", m_id, other.get_id());
            return false;
        }

        if (m_sources_hash.size() != other.m_sources_hash.size() || m_destinations_hash.size() != other.m_destinations_hash.size())
        {
            log_debug("net", "the nets with IDs {} and {} are not equal due to an unequal number of sources or destinations.", m_id, other.get_id());
            return false;
        }

        for (auto ep_n1_it = m_sources_hash.begin(); ep_n1_it != m_sources_hash.end(); ++ep_n1_it)
        {
            if (other.m_sources_hash.find(ep_n1_it->second.get()) == other.m_sources_hash.end())
            {
                log_debug("net", "the nets with IDs {} and {} are not equal due to an unequal source endpoint.", m_id, other.get_id());
                return false;
            }
        }

        for (auto ep_n1_it = m_destinations_hash.begin(); ep_n1_it != m_destinations_hash.end(); ++ep_n1_it)
        {
            if (other.m_destinations_hash.find(ep_n1_it->second.get()) == other.m_destinations_hash.end())
            {
                log_debug("net", "the nets with IDs {} and {} are not equal due to an unequal destination endpoint.", m_id, other.get_id());
                return false;
            }
        }

        if (!DataContainer::operator==(other))
        {
            log_debug("net", "the nets with IDs {} and {} are not equal due to unequal data.", m_id, other.get_id());
            return false;
        }

        return true;
    }

    bool Net::operator!=(const Net& other) const
    {
        return !operator==(other);
    }

    ssize_t Net::get_hash() const
    {
        return (uintptr_t)this;
    }

    u32 Net::get_id() const
    {
        return m_id;
    }

    Netlist* Net::get_netlist() const
    {
        return m_internal_manager->m_netlist;
    }

    const std::string& Net::get_name() const
    {
        return m_name;
    }

    void Net::set_name(const std::string& name)
    {
        if (utils::trim(name).empty())
        {
            log_error("net", "net name cannot be empty.");
            return;
        }
        if (name != m_name)
        {
            m_name = name;
            m_event_handler->notify(NetEvent::event::name_changed, this);
        }
    }

    Grouping* Net::get_grouping() const
    {
        return m_grouping;
    }

    Endpoint* Net::add_source(Gate* gate, GatePin* pin)
    {
        return m_internal_manager->net_add_source(this, gate, pin);
    }

    Endpoint* Net::add_source(Gate* gate, const std::string& pin_name)
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not add source to gate: nullptr given for gate");
            return nullptr;
        }
        if (pin_name.empty())
        {
            log_warning("net", "could not add source to gate '{}' with ID {}: empty string provided as pin name", gate->get_name(), gate->get_id());
            return nullptr;
        }
        GatePin* pin = gate->get_type()->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("net", "could not add source to gate '{}' with ID {}: no pin with name '{}' exists", gate->get_name(), gate->get_id(), pin_name);
            return nullptr;
        }
        return add_source(gate, pin);
    }

    bool Net::remove_source(Gate* gate, const GatePin* pin)
    {
        auto it = m_sources_hash.find(EndpointKey(gate,pin));
        if (it != m_sources_hash.end())
        {
            return m_internal_manager->net_remove_source(this, it->second.get());
        }
        return false;
    }

    bool Net::remove_source(Gate* gate, const std::string& pin_name)
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not remove source from gate: nullptr given for gate");
            return false;
        }
        if (pin_name.empty())
        {
            log_warning("net", "could not remove source from gate '{}' with ID {}: empty string provided as pin name", gate->get_name(), gate->get_id());
            return false;
        }
        GatePin* pin = gate->get_type()->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("net", "could not remove source from gate '{}' with ID {}: no pin with name '{}' exists", gate->get_name(), gate->get_id(), pin_name);
            return false;
        }
        return remove_source(gate, pin);
    }

    bool Net::remove_source(Endpoint* ep)
    {
        if (ep == nullptr)
        {
            return false;
        }
        return m_internal_manager->net_remove_source(this, ep);
    }

    bool Net::is_a_source(const Gate* gate) const
    {
        for (const Endpoint* ep : gate->get_fan_out_endpoints())
            if (m_sources_hash.find(EndpointKey(ep)) != m_sources_hash.end())
                return true;

        return false;
    }

    bool Net::is_a_source(const Gate* gate, const GatePin* pin) const
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not check if gate is a source: nullptr given for gate");
            return false;
        }

        if (pin == nullptr)
        {
            log_warning("net", "could not check if gate is a source: nullptr given for pin");
            return false;
        }

        return m_sources_hash.find(EndpointKey(gate,pin)) != m_sources_hash.end();
    }

    bool Net::is_a_source(const Gate* gate, const std::string& pin_name) const
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not check if gate is a source: nullptr given for gate");
            return false;
        }

        if (pin_name.empty())
        {
            log_warning("net", "could not check if gate '{}' with ID {} is a source: empty string provided as pin name", gate->get_name(), gate->get_id());
            return false;
        }

        return is_a_source(gate, gate->get_type()->get_pin_by_name(pin_name));
    }

    bool Net::is_a_source(const Endpoint* ep) const
    {
        if (ep == nullptr)
        {
            return false;
        }
        if (!ep->is_source_pin())
        {
            return false;
        }

        return m_sources_hash.find(EndpointKey(ep)) != m_sources_hash.end();
    }

    u32 Net::get_num_of_sources() const
    {
        return (u32)m_sources_hash.size();
    }

    std::vector<Endpoint*> Net::get_sources(const std::function<bool(Endpoint* ep)>& filter) const
    {
        std::vector<Endpoint*> srcs;
        if (!filter)
        {
            for (auto it = m_sources_hash.begin(); it != m_sources_hash.end(); ++it)
                srcs.push_back(it->second.get());
        }
        else
        {
            for (auto it = m_sources_hash.begin(); it != m_sources_hash.end(); ++it)
            {
                Endpoint* src = it->second.get();
                if (!filter(src)) continue;
                srcs.push_back(src);
            }
        }
        return srcs;
    }

    Endpoint* Net::add_destination(Gate* gate, GatePin* pin)
    {
        return m_internal_manager->net_add_destination(this, gate, pin);
    }

    Endpoint* Net::add_destination(Gate* gate, const std::string& pin_name)
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not add destination to gate: nullptr given for gate");
            return nullptr;
        }
        if (pin_name.empty())
        {
            log_warning("net", "could not add destination to gate '{}' with ID {}: empty string provided as pin name", gate->get_name(), gate->get_id());
            return nullptr;
        }
        GatePin* pin = gate->get_type()->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("net", "could not add destination to gate '{}' with ID {}: no pin with name '{}' exists", gate->get_name(), gate->get_id(), pin_name);
            return nullptr;
        }
        return add_destination(gate, pin);
    }

    bool Net::remove_destination(Gate* gate, const GatePin* pin)
    {
        auto it = m_destinations_hash.find(EndpointKey(gate,pin));
        if (it != m_destinations_hash.end())
        {
            return m_internal_manager->net_remove_destination(this, it->second.get());
        }
        return false;
    }

    bool Net::remove_destination(Gate* gate, const std::string& pin_name)
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not remove destination from gate: nullptr given for gate");
            return false;
        }
        if (pin_name.empty())
        {
            log_warning("net", "could not remove destination from gate '{}' with ID {}: empty string provided as pin name", gate->get_name(), gate->get_id());
            return false;
        }
        GatePin* pin = gate->get_type()->get_pin_by_name(pin_name);
        if (pin == nullptr)
        {
            log_warning("net", "could not remove destination from gate '{}' with ID {}: no pin with name '{}' exists", gate->get_name(), gate->get_id(), pin_name);
            return false;
        }
        return remove_destination(gate, pin);
    }

    bool Net::remove_destination(Endpoint* ep)
    {
        if (ep == nullptr)
        {
            return false;
        }
        return m_internal_manager->net_remove_destination(this, ep);
    }

    bool Net::is_a_destination(const Gate* gate) const
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not check if gate is a destination: nullptr given for gate");
            return false;
        }

        for (const Endpoint* ep : gate->get_fan_in_endpoints())
            if (m_destinations_hash.find(EndpointKey(ep)) != m_destinations_hash.end())
                return true;

        return false;
    }

    bool Net::is_a_destination(const Gate* gate, const GatePin* pin) const
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not check if gate is a destination: nullptr given for gate");
            return false;
        }

        if (pin == nullptr)
        {
            log_warning("net", "could not check if gate is a destination: nullptr given for pin");
            return false;
        }

        return m_destinations_hash.find(EndpointKey(gate,pin)) != m_destinations_hash.end();
    }

    bool Net::is_a_destination(const Gate* gate, const std::string& pin_name) const
    {
        if (gate == nullptr)
        {
            log_warning("net", "could not check if gate is a destination: nullptr given for gate");
            return false;
        }

        if (pin_name.empty())
        {
            log_warning("net", "could not check if gate '{}' with ID {} is a destination: empty string provided as pin name", gate->get_name(), gate->get_id());
            return false;
        }

        return is_a_destination(gate, gate->get_type()->get_pin_by_name(pin_name));
    }

    bool Net::is_a_destination(const Endpoint* ep) const
    {
        if (ep == nullptr)
        {
            return false;
        }
        if (!ep->is_destination_pin())
        {
            return false;
        }

        return m_destinations_hash.find(EndpointKey(ep))!=m_destinations_hash.end();
    }

    u32 Net::get_num_of_destinations() const
    {
        return (u32)m_destinations_hash.size();
    }

    std::vector<Endpoint*> Net::get_destinations(const std::function<bool(Endpoint* ep)>& filter) const
    {
        std::vector<Endpoint*> dsts;
        if (!filter)
        {
            for (auto it = m_destinations_hash.begin(); it != m_destinations_hash.end(); ++it)
                dsts.push_back(it->second.get());
        }
        else
        {
            for (auto it = m_destinations_hash.begin(); it != m_destinations_hash.end(); ++it)
            {
                Endpoint* dst = it->second.get();
                if (!filter(dst)) continue;
                dsts.push_back(dst);
            }
        }
        return dsts;
    }

    bool Net::is_unrouted() const
    {
        return ((m_sources_hash.size() == 0) || (m_destinations_hash.size() == 0));
    }

    bool Net::is_gnd_net() const
    {
        return m_sources_hash.size() == 1 && m_sources_hash.begin()->first.gate->is_gnd_gate();
    }

    bool Net::is_vcc_net() const
    {
        return m_sources_hash.size() == 1 && m_sources_hash.begin()->first.gate->is_vcc_gate();
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

    bool Net::is_global_input_net() const
    {
        return m_internal_manager->m_netlist->is_global_input_net(this);
    }

    bool Net::is_global_output_net() const
    {
        return m_internal_manager->m_netlist->is_global_output_net(this);
    }
}    // namespace hal
