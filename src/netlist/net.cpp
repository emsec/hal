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

        if (m_sources.size() != other.get_num_of_sources() || m_destinations.size() != other.get_num_of_destinations())
        {
            log_debug("net", "the nets with IDs {} and {} are not equal due to an unequal number of sources or destinations.", m_id, other.get_id());
            return false;
        }

        const std::vector<Endpoint*>& sources_n2 = other.get_sources();
        for (const Endpoint* ep_n1 : m_sources_raw)
        {
            if (std::find_if(sources_n2.begin(), sources_n2.end(), [ep_n1](const Endpoint* ep_n2) { return *ep_n1->get_pin() == *ep_n2->get_pin() && *ep_n1->get_gate() == *ep_n2->get_gate(); })
                == sources_n2.end())
            {
                log_debug("net", "the nets with IDs {} and {} are not equal due to an unequal source endpoint.", m_id, other.get_id());
                return false;
            }
        }

        const std::vector<Endpoint*>& destinations_n2 = other.get_destinations();
        for (const Endpoint* ep_n1 : m_destinations_raw)
        {
            if (std::find_if(
                    destinations_n2.begin(), destinations_n2.end(), [ep_n1](const Endpoint* ep_n2) { return *ep_n1->get_pin() == *ep_n2->get_pin() && *ep_n1->get_gate() == *ep_n2->get_gate(); })
                == destinations_n2.end())
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
        if (auto it = std::find_if(m_sources_raw.begin(), m_sources_raw.end(), [gate, pin](auto ep) { return ep->get_gate() == gate && *ep->get_pin() == *pin; }); it != m_sources_raw.end())
        {
            return m_internal_manager->net_remove_source(this, *it);
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
        if (gate == nullptr)
        {
            log_warning("net", "could not check if gate is a source: nullptr given for gate");
            return false;
        }

        return std::find_if(m_sources_raw.begin(), m_sources_raw.end(), [gate](const auto* ep) { return ep->get_gate() == gate; }) != m_sources_raw.end();
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

        return std::find_if(m_sources_raw.begin(), m_sources_raw.end(), [gate, pin](const auto* ep) { return ep->get_gate() == gate && *ep->get_pin() == *pin; }) != m_sources_raw.end();
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

        return std::find(m_sources_raw.begin(), m_sources_raw.end(), ep) != m_sources_raw.end();
    }

    int Net::get_source_index(const Endpoint* ep) const
    {
        for (int inx = 0; inx < m_sources_raw.size(); inx++)
        {
            if (m_sources_raw.at(inx) == ep) return inx;
        }
        return -1;
    }

    u32 Net::get_num_of_sources(const std::function<bool(Endpoint* ep)>& filter) const
    {
        if (!filter)
        {
            return (u32)m_sources_raw.size();
        }

        u32 num = 0;
        for (auto dst : m_sources_raw)
        {
            if (filter(dst))
            {
                num++;
            }
        }
        return num;
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
        if (auto it = std::find_if(m_destinations_raw.begin(), m_destinations_raw.end(), [gate, pin](const auto* ep) { return ep->get_gate() == gate && *ep->get_pin() == *pin; });
            it != m_destinations_raw.end())
        {
            return m_internal_manager->net_remove_destination(this, *it);
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

        return std::find_if(m_destinations_raw.begin(), m_destinations_raw.end(), [gate](const auto* ep) { return ep->get_gate() == gate; }) != m_destinations_raw.end();
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

        return std::find_if(m_destinations_raw.begin(), m_destinations_raw.end(), [gate, pin](const auto* ep) { return ep->get_gate() == gate && *ep->get_pin() == *pin; }) != m_destinations_raw.end();
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

        return std::find(m_destinations_raw.begin(), m_destinations_raw.end(), ep) != m_destinations_raw.end();
    }

    int Net::get_destination_index(const Endpoint* ep) const
    {
        for (int inx = 0; inx < m_destinations_raw.size(); inx++)
        {
            if (m_destinations_raw.at(inx) == ep) return inx;
        }
        return -1;
    }

    u32 Net::get_num_of_destinations(const std::function<bool(Endpoint* ep)>& filter) const
    {
        if (!filter)
        {
            return (u32)m_destinations_raw.size();
        }

        u32 num = 0;
        for (auto dst : m_destinations_raw)
        {
            if (filter(dst))
            {
                num++;
            }
        }
        return num;
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
        return ((m_sources.size() == 0) || (m_destinations.size() == 0));
    }

    bool Net::is_gnd_net() const
    {
        return m_sources.size() == 1 && m_sources.front()->get_gate()->is_gnd_gate();
    }

    bool Net::is_vcc_net() const
    {
        return m_sources.size() == 1 && m_sources.front()->get_gate()->is_vcc_gate();
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
