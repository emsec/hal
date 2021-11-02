#include "hal_core/netlist/module.h"

#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    Module::Module(NetlistInternalManager* internal_manager, EventHandler* event_handler, u32 id, Module* parent, const std::string& name)
    {
        m_internal_manager = internal_manager;
        m_id               = id;
        m_parent           = parent;
        m_name             = name;

        m_event_handler = event_handler;
    }

    bool Module::operator==(const Module& other) const
    {
        if (m_id != other.get_id() || m_name != other.get_name() || m_type != other.get_type())
        {
            log_info("module", "the modules with IDs {} and {} are not equal due to an unequal ID, name, or type.", m_id, other.get_id());
            return false;
        }

        // does not check parent module to avoid infinite loop

        for (Module* other_module : other.get_submodules())
        {
            if (const auto it = m_submodules_map.find(other_module->get_id()); it == m_submodules_map.end() || *it->second != *other_module)
            {
                log_info("module", "the modules with IDs {} and {} are not equal due to an unequal submodules.", m_id, other.get_id());
                return false;
            }
        }

        for (Gate* other_gate : other.get_gates())
        {
            if (const auto it = m_gates_map.find(other_gate->get_id()); it == m_gates_map.end() || *it->second != *other_gate)
            {
                log_info("module", "the modules with IDs {} and {} are not equal due to an unequal gates.", m_id, other.get_id());
                return false;
            }
        }

        for (const Port* port : get_ports())
        {
            if (const Port* other_port = other.get_port(other.get_netlist()->get_net_by_id(port->get_nets().front()->get_id())); other_port == nullptr || *other_port != *port)
            {
                log_info("module", "the modules with IDs {} and {} are not equal due to an unequal port.", m_id, other.get_id());
                return false;
            }
        }

        if (!DataContainer::operator==(other))
        {
            log_info("module", "the modules with IDs {} and {} are not equal due to unequal data.", m_id, other.get_id());
            return false;
        }

        return true;
    }

    bool Module::operator!=(const Module& other) const
    {
        return !operator==(other);
    }

    u32 Module::get_id() const
    {
        return m_id;
    }

    std::string Module::get_name() const
    {
        return m_name;
    }

    void Module::set_name(const std::string& name)
    {
        if (utils::trim(name).empty())
        {
            log_error("module", "module name cannot be empty.");
            return;
        }
        if (name != m_name)
        {
            m_name = name;

            m_event_handler->notify(ModuleEvent::event::name_changed, this);
        }
    }

    std::string Module::get_type() const
    {
        return m_type;
    }

    void Module::set_type(const std::string& type)
    {
        if (type != m_type)
        {
            m_type = type;

            m_event_handler->notify(ModuleEvent::event::type_changed, this);
        }
    }

    Grouping* Module::get_grouping() const
    {
        return m_grouping;
    }

    Module* Module::get_parent_module() const
    {
        return m_parent;
    }

    std::vector<Module*> Module::get_parent_modules(const std::function<bool(Module*)>& filter, bool recursive) const
    {
        std::vector<Module*> res;
        if (m_parent == nullptr)
        {
            return {};
        }

        if (!filter)
        {
            res.push_back(m_parent);
        }
        else
        {
            if (filter(m_parent))
            {
                res.push_back(m_parent);
            }
        }

        if (recursive)
        {
            std::vector<Module*> more = m_parent->get_parent_modules(filter, true);
            res.reserve(res.size() + more.size());
            res.insert(res.end(), more.begin(), more.end());
        }
        return res;
    }

    int Module::get_submodule_depth() const
    {
        int retval      = 0;
        const Module* p = this;
        while ((p = p->get_parent_module()))
            ++retval;
        return retval;
    }

    bool Module::set_parent_module(Module* new_parent)
    {
        if (new_parent == this)
        {
            log_error("module", "module '{}' with ID {} in netlist with ID {} cannot be its own parent module.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (m_parent == nullptr)
        {
            log_error("module", "no parent module can be assigned to top module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (new_parent == nullptr)
        {
            log_error("module", "module '{}' with ID {} in netlist with ID {} cannot be assigned to be the top module.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (!get_netlist()->is_module_in_netlist(new_parent))
        {
            log_error("module", "module '{}' with ID {} is not contained in netlist with ID {}.", new_parent->get_name(), new_parent->get_id(), m_internal_manager->m_netlist->get_id());
            return false;
        }

        auto children = get_submodules(nullptr, true);
        if (std::find(children.begin(), children.end(), new_parent) != children.end())
        {
            new_parent->set_parent_module(m_parent);
        }

        m_parent->m_submodules_map.erase(m_id);
        m_parent->m_submodules.erase(std::find(m_parent->m_submodules.begin(), m_parent->m_submodules.end(), this));

        for (Net* net : m_parent->get_nets(nullptr, true))
        {
            m_parent->check_net(net);
        }

        m_event_handler->notify(ModuleEvent::event::submodule_removed, m_parent, m_id);

        m_parent = new_parent;

        m_parent->m_submodules_map[m_id] = this;
        m_parent->m_submodules.push_back(this);

        for (Net* net : m_parent->get_nets(nullptr, true))
        {
            m_parent->check_net(net);
        }

        m_event_handler->notify(ModuleEvent::event::parent_changed, this);
        m_event_handler->notify(ModuleEvent::event::submodule_added, m_parent, m_id);

        return true;
    }

    bool Module::is_parent_module_of(Module* module, bool recursive) const
    {
        if (!module->get_parent_modules([this](Module* m) { return this->get_id() == m->get_id(); }, recursive).empty())
        {
            return true;
        }

        return false;
    }

    std::vector<Module*> Module::get_submodules(const std::function<bool(Module*)>& filter, bool recursive) const
    {
        std::vector<Module*> res;
        if (!filter)
        {
            res = m_submodules;
        }
        else
        {
            for (auto sm : m_submodules)
            {
                if (filter(sm))
                {
                    res.push_back(sm);
                }
            }
        }

        if (recursive)
        {
            for (auto sm : m_submodules)
            {
                auto more = sm->get_submodules(filter, true);
                res.reserve(res.size() + more.size());
                res.insert(res.end(), more.begin(), more.end());
            }
        }
        return res;
    }

    bool Module::is_submodule_of(Module* module, bool recursive) const
    {
        if (!get_parent_modules([module](Module* m) { return module->get_id() == m->get_id(); }, recursive).empty())
        {
            return true;
        }

        return false;
    }

    bool Module::contains_module(Module* other, bool recursive) const
    {
        if (other == nullptr)
        {
            return false;
        }
        for (auto sm : m_submodules)
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

    bool Module::is_top_module() const
    {
        return m_parent == nullptr;
    }

    Netlist* Module::get_netlist() const
    {
        return m_internal_manager->m_netlist;
    }

    bool Module::assign_gate(Gate* gate)
    {
        return m_internal_manager->module_assign_gates(this, {gate});
    }

    bool Module::assign_gates(const std::vector<Gate*>& gates)
    {
        return m_internal_manager->module_assign_gates(this, gates);
    }

    bool Module::remove_gate(Gate* gate)
    {
        return remove_gates({gate});
    }

    bool Module::remove_gates(const std::vector<Gate*>& gates)
    {
        for (Gate* gate : gates)
        {
            if (gate == nullptr || !contains_gate(gate))
            {
                return false;
            }
        }

        return m_internal_manager->module_assign_gates(m_internal_manager->m_netlist->get_top_module(), gates);
    }

    bool Module::contains_gate(Gate* gate, bool recursive) const
    {
        if (gate == nullptr)
        {
            return false;
        }
        bool success = std::find(m_gates.begin(), m_gates.end(), gate) != m_gates.end();
        if (!success && recursive)
        {
            for (auto sm : m_submodules)
            {
                if (sm->contains_gate(gate, true))
                {
                    return true;
                }
            }
        }
        return success;
    }

    Gate* Module::get_gate_by_id(const u32 gate_id, bool recursive) const
    {
        auto it = m_gates_map.find(gate_id);
        if (it == m_gates_map.end())
        {
            if (recursive)
            {
                for (auto sm : m_submodules)
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

    std::vector<Gate*> Module::get_gates(const std::function<bool(Gate*)>& filter, bool recursive) const
    {
        std::vector<Gate*> res;
        if (!filter)
        {
            res = m_gates;
        }
        else
        {
            for (auto g : m_gates)
            {
                if (!filter(g))
                {
                    continue;
                }
                res.push_back(g);
            }
        }

        if (recursive)
        {
            for (auto sm : m_submodules)
            {
                auto more = sm->get_gates(filter, true);
                res.reserve(res.size() + more.size());
                res.insert(res.end(), more.begin(), more.end());
            }
        }

        return res;
    }

    /*
     * ################################################################
     *      net functions
     * ################################################################
     */

    bool Module::contains_net(Net* net, bool recursive) const
    {
        if (net == nullptr)
        {
            return false;
        }
        bool success = m_nets.find(net) != m_nets.end();
        if (!success && recursive)
        {
            for (auto sm : m_submodules)
            {
                if (sm->contains_net(net, true))
                {
                    return true;
                }
            }
        }
        return success;
    }

    std::vector<Net*> Module::get_nets(const std::function<bool(Net*)>& filter, bool recursive) const
    {
        std::vector<Net*> res;
        if (!filter)
        {
            res = std::vector<Net*>(m_nets.begin(), m_nets.end());
        }
        else
        {
            for (Net* n : m_nets)
            {
                if (!filter(n))
                {
                    continue;
                }
                res.push_back(n);
            }
        }

        if (recursive)
        {
            for (auto sm : m_submodules)
            {
                auto more = sm->get_nets(filter, true);
                res.reserve(res.size() + more.size());
                res.insert(res.end(), more.begin(), more.end());
            }
        }

        return res;
    }

    std::vector<Net*> Module::get_input_nets() const
    {
        return std::vector<Net*>(m_input_nets.begin(), m_input_nets.end());
    }

    std::vector<Net*> Module::get_output_nets() const
    {
        return std::vector<Net*>(m_output_nets.begin(), m_output_nets.end());
    }

    std::vector<Net*> Module::get_internal_nets() const
    {
        return std::vector<Net*>(m_internal_nets.begin(), m_internal_nets.end());
    }

    bool Module::is_input_net(Net* net) const
    {
        if (net == nullptr)
        {
            return false;
        }

        return m_input_nets.find(net) != m_input_nets.end();
    }

    bool Module::is_output_net(Net* net) const
    {
        if (net == nullptr)
        {
            return false;
        }

        return m_output_nets.find(net) != m_output_nets.end();
    }

    bool Module::is_internal_net(Net* net) const
    {
        if (net == nullptr)
        {
            return false;
        }

        return m_internal_nets.find(net) != m_internal_nets.end();
    }

    void Module::check_net(Net* net, bool recursive)
    {
        std::vector<Endpoint*> sources      = net->get_sources();
        std::vector<Endpoint*> destinations = net->get_destinations();

        bool external_source      = net->is_global_input_net();
        bool internal_source      = false;
        bool external_destination = net->is_global_output_net();
        bool internal_destination = false;

        for (Endpoint* ep : sources)
        {
            if (Module* mod = ep->get_gate()->get_module(); this != mod && !is_parent_module_of(mod))
            {
                external_source = true;
            }
            else
            {
                internal_source = true;
            }

            if (external_source && internal_source)
            {
                break;
            }
        }

        for (Endpoint* ep : destinations)
        {
            if (Module* mod = ep->get_gate()->get_module(); this != mod && !is_parent_module_of(mod))
            {
                external_destination = true;
            }
            else
            {
                internal_destination = true;
            }

            if (external_destination && internal_destination)
            {
                break;
            }
        }

        if (internal_source && internal_destination && external_source && external_destination)
        {
            if (m_input_nets.find(net) == m_input_nets.end() || m_output_nets.find(net) == m_output_nets.end())
            {
                m_input_nets.insert(net);
                m_output_nets.insert(net);
                if (Port* port = get_port(net); port != nullptr)
                {
                    port->m_direction = PinDirection::inout;
                }
                else
                {
                    assign_port_net(net, PinDirection::inout);
                }
                m_event_handler->notify(ModuleEvent::event::port_changed, this, net->get_id());
            }
        }
        else
        {
            if (external_source && internal_destination)
            {
                if (m_input_nets.find(net) == m_input_nets.end())
                {
                    m_input_nets.insert(net);
                    if (Port* port = get_port(net); port != nullptr)
                    {
                        port->m_direction = PinDirection::input;
                    }
                    else
                    {
                        assign_port_net(net, PinDirection::input);
                    }
                    m_event_handler->notify(ModuleEvent::event::port_changed, this, net->get_id());
                }
            }
            else
            {
                if (m_input_nets.find(net) != m_input_nets.end())
                {
                    m_input_nets.erase(net);
                    remove_port_net(net);
                    m_event_handler->notify(ModuleEvent::event::port_changed, this, net->get_id());
                }
            }

            if (external_destination && internal_source)
            {
                if (m_output_nets.find(net) == m_output_nets.end())
                {
                    m_output_nets.insert(net);
                    if (Port* port = get_port(net); port != nullptr)
                    {
                        port->m_direction = PinDirection::output;
                    }
                    else
                    {
                        assign_port_net(net, PinDirection::output);
                    }
                    m_event_handler->notify(ModuleEvent::event::port_changed, this, net->get_id());
                }
            }
            else
            {
                if (m_output_nets.find(net) != m_output_nets.end())
                {
                    m_output_nets.erase(net);
                    remove_port_net(net);
                }
                m_event_handler->notify(ModuleEvent::event::port_changed, this, net->get_id());
            }
        }

        if (internal_source && internal_destination)
        {
            m_internal_nets.insert(net);
        }
        else
        {
            if (internal_source || internal_destination)
            {
                m_nets.insert(net);
            }
            m_internal_nets.erase(net);
        }

        if (recursive && m_parent != nullptr)
        {
            m_parent->check_net(net, true);
        }
    }

    /*
     * ################################################################
     *      port functions
     * ################################################################
     */

    Module::Port::Port(Module* module, const std::string& name, Net* net, PinDirection direction, PinType type) : m_module(module), m_name(name), m_direction(direction), m_type(type)
    {
        m_pins.push_back(std::make_pair(name, net));
        m_pin_to_net[name] = net;
        m_net_to_pin[net]  = name;
    }

    Module::Port::Port(Module* module, const std::string& name, const std::vector<std::pair<std::string, Net*>>& pins_and_nets, PinDirection direction, PinType type)
        : m_module(module), m_name(name), m_direction(direction), m_type(type)
    {
        for (const std::pair<std::string, Net*>& pin : pins_and_nets)
        {
            m_pins.push_back(pin);
            m_pin_to_net[pin.first]  = pin.second;
            m_net_to_pin[pin.second] = pin.first;
        }
    }

    bool Module::Port::operator==(const Port& other) const
    {
        if (m_name != other.get_name() || m_type != other.get_type() || m_direction != other.get_direction())
        {
            return false;
        }

        auto other_it = other.get_pins_and_nets().begin();
        for (auto this_it = m_pins.begin(); this_it != m_pins.end(); this_it++, other_it++)
        {
            if (this_it->first != other_it->first || *this_it->second != *other_it->second)
            {
                return false;
            }
        }

        return true;
    }

    bool Module::Port::operator!=(const Port& other) const
    {
        return !operator==(other);
    }

    Module* Module::Port::get_module() const
    {
        return m_module;
    }

    const std::string& Module::Port::get_name() const
    {
        return m_name;
    }

    PinDirection Module::Port::get_direction() const
    {
        return m_direction;
    }

    PinType Module::Port::get_type() const
    {
        return m_type;
    }

    std::vector<std::string> Module::Port::get_pins() const
    {
        std::vector<std::string> res;
        res.reserve(m_pins.size());
        for (const std::pair<std::string, Net*>& pin : m_pins)
        {
            res.push_back(pin.first);
        }
        return res;
    }

    std::vector<Net*> Module::Port::get_nets() const
    {
        std::vector<Net*> res;
        res.reserve(m_pins.size());
        for (const std::pair<std::string, Net*>& pin : m_pins)
        {
            res.push_back(pin.second);
        }
        return res;
    }

    std::string Module::Port::get_pin(Net* net) const
    {
        if (net == nullptr)
        {
            return "";
        }

        if (const auto it = m_net_to_pin.find(net); it != m_net_to_pin.end())
        {
            return it->second;
        }

        return "";
    }

    Net* Module::Port::get_net(const std::string& pin_name) const
    {
        if (pin_name.empty())
        {
            return nullptr;
        }

        if (const auto it = m_pin_to_net.find(pin_name); it != m_pin_to_net.end())
        {
            return it->second;
        }

        return nullptr;
    }

    std::vector<std::pair<std::string, Net*>> Module::Port::get_pins_and_nets() const
    {
        std::vector<std::pair<std::string, Net*>> res;
        res.reserve(m_pins.size());
        for (const std::pair<std::string, Net*>& pin : m_pins)
        {
            res.push_back(pin);
        }
        return res;
    }

    bool Module::Port::is_multi_bit() const
    {
        return m_pins.size() > 1;
    }

    bool Module::Port::contains_pin(const std::string& pin_name) const
    {
        return m_pin_to_net.find(pin_name) != m_pin_to_net.end();
    }

    bool Module::Port::contains_net(Net* net) const
    {
        return m_net_to_pin.find(net) != m_net_to_pin.end();
    }

    bool Module::Port::move_pin(const std::string& pin_name, u32 new_index)
    {
        if (const auto pin_it = std::find_if(m_pins.begin(), m_pins.end(), [pin_name](const std::pair<std::string, Net*>& pin) { return pin.first == pin_name; }); pin_it != m_pins.end())
        {
            Net* net = pin_it->second;
            m_pins.erase(pin_it);

            auto it = m_pins.begin();
            std::advance(it, new_index);
            m_pins.insert(it, std::make_pair(pin_name, net));
        }
        return false;
    }

    bool Module::Port::move_pin(Net* net, u32 new_index)
    {
        if (const auto it = m_net_to_pin.find(net); it != m_net_to_pin.end())
        {
            return move_pin(it->second, new_index);
        }
        return false;
    }

    void Module::set_next_input_port_id(u32 id)
    {
        m_next_input_index = id;
    }

    void Module::set_next_inout_port_id(u32 id)
    {
        m_next_inout_index = id;
    }

    void Module::set_next_output_port_id(u32 id)
    {
        m_next_output_index = id;
    }

    u32 Module::get_next_input_port_id() const
    {
        return m_next_input_index;
    }

    u32 Module::get_next_inout_port_id() const
    {
        return m_next_inout_index;
    }

    u32 Module::get_next_output_port_id() const
    {
        return m_next_input_index;
    }

    std::vector<Module::Port*> Module::get_ports(const std::function<bool(Port*)>& filter) const
    {
        std::vector<Port*> res;
        if (!filter)
        {
            res.reserve(m_ports_raw.size());
            res = std::vector<Port*>(m_ports_raw.begin(), m_ports_raw.end());
        }
        else
        {
            for (Port* port : m_ports_raw)
            {
                if (!filter(port))
                {
                    continue;
                }
                res.push_back(port);
            }
        }

        return res;
    }

    Module::Port* Module::get_port(const std::string& port_name) const
    {
        if (port_name.empty())
        {
            log_warning("module", "empty port name passed to get port of module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        if (const auto it = m_port_names_map.find(port_name); it != m_port_names_map.end())
        {
            return it->second;
        }

        log_debug("module", "'{}' is not a port of module '{}' with ID {} in netlist with ID {}.", port_name, m_name, m_id, m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    Module::Port* Module::get_port(Net* port_net) const
    {
        if (port_net == nullptr)
        {
            log_warning("module", "nullptr given as port net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        if (const auto it = std::find_if(m_ports_raw.begin(), m_ports_raw.end(), [port_net](Port* port) { return port->contains_net(port_net); }); it != m_ports_raw.end())
        {
            return *it;
        }

        log_debug("module",
                  "net '{}' with ID {} is not connected to a port of module '{}' with ID {} in netlist with ID {}.",
                  port_net->get_name(),
                  port_net->get_id(),
                  m_name,
                  m_id,
                  m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    Module::Port* Module::get_port_by_pin_name(const std::string& pin_name) const
    {
        if (pin_name.empty())
        {
            log_warning("module", "empty pin name passed to get port of module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        if (const auto it = m_pin_names_map.find(pin_name); it != m_pin_names_map.end())
        {
            return it->second;
        }

        log_debug("module", "'{}' is not a pin of a port of module '{}' with ID {} in netlist with ID {}.", pin_name, m_name, m_id, m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    bool Module::set_port_name(Port* port, const std::string& new_name)
    {
        if (port == nullptr || port->get_module() != this || new_name.empty())
        {
            return false;
        }

        if (m_port_names_map.find(new_name) != m_port_names_map.end() || m_pin_names_map.find(new_name) != m_pin_names_map.end()
            || std::find(m_ports_raw.begin(), m_ports_raw.end(), port) == m_ports_raw.end())
        {
            return false;
        }

        std::string old_name = port->m_name;
        port->m_name         = new_name;
        m_port_names_map.erase(old_name);
        m_port_names_map[new_name] = port;

        if (port->m_pins.size() == 1)
        {
            std::get<0>(port->m_pins.front()) = new_name;
            m_pin_names_map.erase(old_name);
            m_pin_names_map[new_name] = port;
        }

        return true;
    }

    bool Module::set_port_type(Port* port, PinType new_type)
    {
        if (port == nullptr || port->get_module() != this)
        {
            return false;
        }

        port->m_type = new_type;
        return true;
    }

    bool Module::set_port_pin_name(Port* port, const std::string& old_name, const std::string& new_name)
    {
        if (port == nullptr || port->get_module() != this || old_name.empty() || new_name.empty())
        {
            return false;
        }

        if (m_port_names_map.find(new_name) != m_port_names_map.end() || m_pin_names_map.find(new_name) != m_pin_names_map.end()
            || std::find(m_ports_raw.begin(), m_ports_raw.end(), port) == m_ports_raw.end())
        {
            return false;
        }

        if (const auto net_it = port->m_pin_to_net.find(old_name); net_it != port->m_pin_to_net.end())
        {
            if (const auto pin_it = std::find_if(port->m_pins.begin(), port->m_pins.end(), [old_name](const std::pair<std::string, Net*>& pin) { return pin.first == old_name; });
                pin_it != port->m_pins.end())
            {
                Net* net = net_it->second;
                port->m_pin_to_net.erase(net_it);
                port->m_pin_to_net[new_name] = net;
                port->m_net_to_pin[net]      = new_name;
                std::get<0>(*pin_it)         = new_name;
                return true;
            }
        }

        return false;
    }

    bool Module::set_port_pin_name(Port* port, Net* net, const std::string& new_name)
    {
        if (port == nullptr || port->get_module() != this || net == nullptr || new_name.empty())
        {
            return false;
        }

        if (const auto it = port->m_net_to_pin.find(net); it != port->m_net_to_pin.end())
        {
            return set_port_pin_name(port, it->second, new_name);
        }

        return false;
    }

    Module::Port* Module::assign_port_net(Net* net, PinDirection direction)
    {
        std::string port_prefix;
        u32* index_counter;
        switch (direction)
        {
            case PinDirection::input:
                port_prefix   = "I";
                index_counter = &m_next_input_index;
                break;
            case PinDirection::inout:
                port_prefix   = "IO";
                index_counter = &m_next_inout_index;
                break;
            case PinDirection::output:
                port_prefix   = "O";
                index_counter = &m_next_output_index;
                break;
            default:
                return nullptr;
        }

        std::string port_name;
        do
        {
            port_name = port_prefix + "(" + std::to_string((*index_counter)++) + ")";
        } while (m_port_names_map.find(port_name) != m_port_names_map.end());

        std::unique_ptr<Port> port_owner(new Port(this, port_name, net, direction));
        Port* new_port = port_owner.get();
        m_ports.push_back(std::move(port_owner));
        m_ports_raw.push_back(new_port);
        m_port_names_map[port_name] = new_port;
        m_pin_names_map[port_name]  = new_port;
        return new_port;
    }

    bool Module::remove_port_net(Net* net)
    {
        Port* port = get_port(net);
        if (port == nullptr)
        {
            return false;
        }

        if (const auto it = port->m_net_to_pin.find(net); it != port->m_net_to_pin.end())
        {
            std::string pin = it->second;
            port->m_net_to_pin.erase(it);
            port->m_pin_to_net.erase(pin);
            port->m_pins.erase(std::find_if(port->m_pins.begin(), port->m_pins.end(), [pin](std::pair<std::string, Net*> pin_net) { return pin_net.first == pin; }));
            m_pin_names_map.erase(pin);
        }
        else
        {
            return false;
        }

        if (port->m_pins.empty())
        {
            m_port_names_map.erase(port->get_name());
            m_ports_raw.erase(std::find(m_ports_raw.begin(), m_ports_raw.end(), port));
            m_ports.erase(std::find_if(m_ports.begin(), m_ports.end(), [port](const std::unique_ptr<Port>& p) { return p.get() == port; }));
        }

        return true;
    }

    /**
     * Merge multiple existing ports into a single multi-bit port.
     * 
     * @param[in] name - The name of the new port.
     * @param[in] ports_to_merge - The ports to be merged in the order in which they should be assigned to the new port.
     */
    Module::Port* Module::create_multi_bit_port(const std::string& name, const std::vector<Port*>& ports_to_merge)
    {
        if (name.empty() || m_port_names_map.find(name) != m_port_names_map.end())
        {
            return nullptr;
        }

        Port* first_port       = ports_to_merge.front();
        PinDirection direction = first_port->m_direction;
        PinType type           = first_port->m_type;

        std::vector<std::pair<std::string, Net*>> pins_and_nets;
        for (Port* port : ports_to_merge)
        {
            if (port == nullptr || port->get_module() != this || port->m_direction != direction || port->m_type != type)
            {
                return nullptr;
            }

            if (std::find(m_ports_raw.begin(), m_ports_raw.end(), port) == m_ports_raw.end())
            {
                return nullptr;
            }

            for (const auto& [pin_name, net] : port->m_pins)
            {
                pins_and_nets.push_back(std::make_pair(pin_name, net));
            }
        }

        // create multi-bit port
        std::unique_ptr<Port> port_owner(new Port(this, name, pins_and_nets, direction, type));
        Port* new_port = port_owner.get();
        m_ports.push_back(std::move(port_owner));
        m_ports_raw.push_back(new_port);
        m_port_names_map[name] = new_port;
        for (const std::pair<std::string, Net*>& pin : pins_and_nets)
        {
            m_pin_names_map.at(pin.first) = new_port;
        }

        // delete old ports
        for (Port* port : ports_to_merge)
        {
            m_port_names_map.erase(port->m_name);
            m_ports_raw.erase(std::find(m_ports_raw.begin(), m_ports_raw.end(), port));
            m_ports.erase(std::find_if(m_ports.begin(), m_ports.end(), [port](const std::unique_ptr<Port>& p) { return p.get() == port; }));
        }

        return new_port;
    }

    /**
     * Split a multi-bit port into multiple single-bit ports.
     * 
     * @param[in] port - The port to be split.
     */
    bool Module::delete_multi_bit_port(Port* port)
    {
        if (port == nullptr || port->get_module() != this || std::find(m_ports_raw.begin(), m_ports_raw.end(), port) == m_ports_raw.end())
        {
            return false;
        }

        // create ports
        for (const auto& [pin_name, net] : port->m_pins)
        {
            std::unique_ptr<Port> port_owner(new Port(this, pin_name, net, port->m_direction, port->m_type));
            Port* new_port = port_owner.get();
            m_ports.push_back(std::move(port_owner));
            m_ports_raw.push_back(new_port);
            m_port_names_map[pin_name]   = new_port;
            m_pin_names_map.at(pin_name) = new_port;
        }

        // delete old multi-bit port
        m_port_names_map.erase(port->m_name);
        m_ports_raw.erase(std::find(m_ports_raw.begin(), m_ports_raw.end(), port));
        m_ports.erase(std::find_if(m_ports.begin(), m_ports.end(), [port](const std::unique_ptr<Port>& p) { return p.get() == port; }));

        return true;
    }

}    // namespace hal
