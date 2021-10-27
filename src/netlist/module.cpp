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

        // TODO add checking for ports again
        // for (const Port* port : get_ports())
        // {
        //     if (const Port* other_port = other.get_port(other.get_netlist()->get_net_by_id(port->get_nets().front()->get_id())); other_port == nullptr || *other_port != *port)
        //     {
        //         log_info("module", "the modules with IDs {} and {} are not equal due to an unequal port.", m_id, other.get_id());
        //         return false;
        //     }
        // }

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

        m_parent->set_cache_dirty();

        m_event_handler->notify(ModuleEvent::event::submodule_removed, m_parent, m_id);

        m_parent = new_parent;

        m_parent->m_submodules_map[m_id] = this;
        m_parent->m_submodules.push_back(this);

        m_parent->set_cache_dirty();

        m_event_handler->notify(ModuleEvent::event::parent_changed, this);
        m_event_handler->notify(ModuleEvent::event::submodule_added, m_parent, m_id);

        return true;
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

    void Module::set_cache_dirty(bool is_dirty)
    {
        m_ports_dirty         = is_dirty;
        m_nets_dirty          = is_dirty;
        m_input_nets_dirty    = is_dirty;
        m_output_nets_dirty   = is_dirty;
        m_internal_nets_dirty = is_dirty;
    }

    bool Module::assign_gate(Gate* gate)
    {
        return m_internal_manager->module_assign_gate(this, gate);
    }

    bool Module::remove_gate(Gate* gate)
    {
        if (contains_gate(gate))
        {
            return m_internal_manager->module_assign_gate(m_internal_manager->m_netlist->get_top_module(), gate);
        }

        if (gate == nullptr)
        {
            log_error("module", "gate cannot be a nullptr.");
            return false;
        }

        log_error(
            "module", "gate '{}' with ID {} does not belong to module '{}' with ID {} in netlist with ID {}.", gate->get_name(), gate->get_id(), m_name, m_id, m_internal_manager->m_netlist->get_id());
        return false;
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

    const std::vector<Net*>& Module::get_nets() const
    {
        if (m_nets_dirty)
        {
            std::unordered_set<const Net*> seen;
            m_nets.clear();

            for (const Gate* gate : get_gates())
            {
                for (Net* net : gate->get_fan_in_nets())
                {
                    if (seen.find(net) == seen.end())
                    {
                        m_nets.push_back(net);
                    }
                }

                for (Net* net : gate->get_fan_out_nets())
                {
                    if (seen.find(net) == seen.end())
                    {
                        m_nets.push_back(net);
                    }
                }
            }

            for (const Module* module : get_submodules())
            {
                for (Net* net : module->get_input_nets())
                {
                    if (seen.find(net) == seen.end())
                    {
                        seen.insert(net);
                        m_nets.push_back(net);
                    }
                }

                for (Net* net : module->get_output_nets())
                {
                    if (seen.find(net) == seen.end())
                    {
                        seen.insert(net);
                        m_nets.push_back(net);
                    }
                }
            }

            std::sort(m_nets.begin(), m_nets.end());
            m_nets_dirty = false;
        }

        return m_nets;
    }

    const std::vector<Net*>& Module::get_input_nets() const
    {
        if (m_input_nets_dirty)
        {
            std::unordered_set<Net*> seen;
            m_input_nets.clear();
            auto gates = get_gates(nullptr, true);
            std::sort(gates.begin(), gates.end());
            for (auto gate : gates)
            {
                for (auto net : gate->get_fan_in_nets())
                {
                    if (seen.find(net) != seen.end())
                    {
                        continue;
                    }
                    seen.insert(net);
                    if (m_internal_manager->m_netlist->is_global_input_net(net))
                    {
                        m_input_nets.push_back(net);
                        continue;
                    }
                    auto sources = net->get_sources();
                    if (std::any_of(sources.begin(), sources.end(), [&gates](auto src) { return !std::binary_search(gates.begin(), gates.end(), src->get_gate()); }))
                    {
                        m_input_nets.push_back(net);
                    }
                }
            }
            std::sort(m_input_nets.begin(), m_input_nets.end());
            m_input_nets_dirty = false;
        }
        return m_input_nets;
    }

    const std::vector<Net*>& Module::get_output_nets() const
    {
        if (m_output_nets_dirty)
        {
            std::unordered_set<Net*> seen;
            m_output_nets.clear();
            auto gates = get_gates(nullptr, true);
            std::sort(gates.begin(), gates.end());
            for (auto gate : gates)
            {
                for (auto net : gate->get_fan_out_nets())
                {
                    if (seen.find(net) != seen.end())
                    {
                        continue;
                    }
                    seen.insert(net);
                    if (m_internal_manager->m_netlist->is_global_output_net(net))
                    {
                        m_output_nets.push_back(net);
                        continue;
                    }
                    auto destinations = net->get_destinations();
                    if (std::any_of(destinations.begin(), destinations.end(), [&gates](auto dst) { return !std::binary_search(gates.begin(), gates.end(), dst->get_gate()); }))
                    {
                        m_output_nets.push_back(net);
                    }
                }
            }
            std::sort(m_output_nets.begin(), m_output_nets.end());
            m_output_nets_dirty = false;
        }
        return m_output_nets;
    }

    const std::vector<Net*>& Module::get_internal_nets() const
    {
        if (m_internal_nets_dirty)
        {
            std::unordered_set<Net*> seen;
            m_internal_nets.clear();
            auto gates = get_gates(nullptr, true);
            std::sort(gates.begin(), gates.end());
            for (auto gate : gates)
            {
                for (auto net : gate->get_fan_out_nets())
                {
                    if (seen.find(net) != seen.end())
                    {
                        continue;
                    }
                    seen.insert(net);
                    auto destinations = net->get_destinations();
                    if (std::any_of(destinations.begin(), destinations.end(), [&gates](auto dst) { return std::binary_search(gates.begin(), gates.end(), dst->get_gate()); }))
                    {
                        m_internal_nets.push_back(net);
                    }
                }
            }
            std::sort(m_internal_nets.begin(), m_internal_nets.end());
            m_internal_nets_dirty = false;
        }
        return m_internal_nets;
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

    bool Module::Port::remove_pin(const std::string& pin_name)
    {
        if (const auto net_it = m_pin_to_net.find(pin_name); net_it != m_pin_to_net.end())
        {
            if (const auto pin_it = std::find_if(m_pins.begin(), m_pins.end(), [pin_name](const std::pair<std::string, Net*>& pin) { return pin.first == pin_name; }); pin_it != m_pins.end())
            {
                m_net_to_pin.erase(net_it->second);
                m_pin_to_net.erase(net_it);
                m_pins.erase(pin_it);

                return true;
            }

            return false;
        }
        return false;
    }

    bool Module::Port::remove_pin(Net* net)
    {
        if (const auto it = m_net_to_pin.find(net); it != m_net_to_pin.end())
        {
            return remove_pin(it->second);
        }
        return false;
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

    PinDirection Module::determine_port_direction(Net* net) const
    {
        PinDirection direction = PinDirection::none;

        const std::vector<Net*>& input_nets  = get_input_nets();
        const std::vector<Net*>& output_nets = get_output_nets();

        if (std::find(input_nets.begin(), input_nets.end(), net) != input_nets.end())
        {
            direction = PinDirection::input;
        }
        if (std::find(output_nets.begin(), output_nets.end(), net) != output_nets.end())
        {
            if (direction == PinDirection::input)
            {
                direction = PinDirection::inout;
            }
            else
            {
                direction = PinDirection::output;
            }
        }

        return direction;
    }

    void Module::update_ports()
    {
        if (m_ports_dirty)
        {
            const std::vector<Net*>& input_nets  = get_input_nets();
            const std::vector<Net*>& output_nets = get_output_nets();

            std::set<Net*> current_port_nets;
            current_port_nets.insert(input_nets.begin(), input_nets.end());
            current_port_nets.insert(output_nets.begin(), output_nets.end());

            std::vector<Net*> diff_nets;

            // find nets that are still in the port map but no longer a port net
            std::set_difference(m_port_nets.begin(), m_port_nets.end(), current_port_nets.begin(), current_port_nets.end(), std::back_inserter(diff_nets));
            for (Net* port_net : diff_nets)
            {
                Port* old_port = get_port(port_net);
                for (const std::string& pin : old_port->get_pins())
                {
                    m_pin_names_map.erase(pin);
                }

                m_port_names_map.erase(old_port->get_name());

                m_ports_raw.erase(std::find(m_ports_raw.begin(), m_ports_raw.end(), old_port));
                m_ports.erase(std::find_if(m_ports.begin(), m_ports.end(), [old_port](const std::unique_ptr<Port>& p) { return p.get() == old_port; }));
            }

            diff_nets.clear();

            // update port directions
            for (Port* port : m_ports_raw)
            {
                port->m_direction = determine_port_direction(port->get_nets().front());
            }

            // find nets that are port nets but have not yet been assigned a port name
            std::set_difference(current_port_nets.begin(), current_port_nets.end(), m_port_nets.begin(), m_port_nets.end(), std::back_inserter(diff_nets));
            for (Net* port_net : diff_nets)
            {
                PinDirection direction = determine_port_direction(port_net);

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
                        continue;
                }

                std::string port_name;
                do
                {
                    port_name = port_prefix + "(" + std::to_string((*index_counter)++) + ")";
                } while (m_port_names_map.find(port_name) != m_port_names_map.end());

                std::unique_ptr<Port> port_owner(new Port(this, port_name, port_net, direction));
                Port* new_port = port_owner.get();
                m_ports.push_back(std::move(port_owner));
                m_ports_raw.push_back(new_port);
                m_port_names_map[port_name] = new_port;
                m_pin_names_map[port_name]  = new_port;
            }

            m_ports_dirty = false;
        }
    }

    std::vector<Module::Port*> Module::get_ports(const std::function<bool(Port*)>& filter)
    {
        update_ports();

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

    Module::Port* Module::get_port(const std::string& port_name)
    {
        if (port_name.empty())
        {
            log_warning("module", "empty port name passed to get port of module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        update_ports();

        if (const auto it = m_port_names_map.find(port_name); it != m_port_names_map.end())
        {
            return it->second;
        }

        log_debug("module", "'{}' is not a port of module '{}' with ID {} in netlist with ID {}.", port_name, m_name, m_id, m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    Module::Port* Module::get_port(Net* port_net)
    {
        if (port_net == nullptr)
        {
            log_warning("module", "nullptr given as port net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        update_ports();

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

    Module::Port* Module::get_port_by_pin_name(const std::string& pin_name)
    {
        if (pin_name.empty())
        {
            log_warning("module", "empty pin name passed to get port of module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        update_ports();

        if (const auto it = std::find_if(m_ports_raw.begin(), m_ports_raw.end(), [pin_name](Port* port) { return port->contains_pin(pin_name); }); it != m_ports_raw.end())
        {
            return *it;
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

        update_ports();

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

        update_ports();

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

        update_ports();

        if (const auto it = port->m_net_to_pin.find(net); it != port->m_net_to_pin.end())
        {
            return set_port_pin_name(port, it->second, new_name);
        }

        return false;
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
