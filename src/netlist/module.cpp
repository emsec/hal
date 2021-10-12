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

        if (other.get_input_port_names().size() != get_input_port_names().size() || other.get_output_port_names().size() != get_output_port_names().size())
        {
            log_info("module", "the modules with IDs {} and {} are not equal due to unequal number of port names.", m_id, other.get_id());
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

        for (const auto& [net, port_name] : get_input_port_names())
        {
            if (const Net* other_net = other.get_input_port_net(port_name); other_net == nullptr || *other_net != *net)
            {
                log_info("module", "the modules with IDs {} and {} are not equal due to an unequal input ports.", m_id, other.get_id());
                return false;
            }
        }

        for (const auto& [net, port_name] : get_output_port_names())
        {
            if (const Net* other_net = other.get_output_port_net(port_name); other_net == nullptr || *other_net != *net)
            {
                log_info("module", "the modules with IDs {} and {} are not equal due to an unequal output ports.", m_id, other.get_id());
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

    bool Module::set_input_port_name(Net* input_net, const std::string& port_name)
    {
        return add_port(input_net, port_name);
    }

    bool Module::set_output_port_name(Net* output_net, const std::string& port_name)
    {
        return add_port(output_net, port_name);
    }

    std::string Module::get_input_port_name(Net* net) const
    {
        if (net == nullptr)
        {
            log_error("module", "nullptr given as input net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return "";
        }

        Port* port = get_port_by_net(net);
        if (port == nullptr || (port->m_direction != PinDirection::input && port->m_direction != PinDirection::inout))
        {
            log_debug("module",
                      "net '{}' with ID {} is not an input net of module '{}' with ID {} in netlist with ID {}.",
                      net->get_name(),
                      net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return "";
        }

        return port->m_name;
    }

    std::string Module::get_output_port_name(Net* net) const
    {
        if (net == nullptr)
        {
            log_error("module", "nullptr given as output net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return "";
        }

        Port* port = get_port_by_net(net);
        if (port == nullptr || (port->m_direction != PinDirection::output && port->m_direction != PinDirection::inout))
        {
            log_debug("module",
                      "net '{}' with ID {} is not an output net of module '{}' with ID {} in netlist with ID {}.",
                      net->get_name(),
                      net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return "";
        }

        return port->m_name;
    }

    Net* Module::get_input_port_net(const std::string& port_name) const
    {
        Port* port = get_port_by_name(port_name);
        if (port == nullptr || (port->m_direction != PinDirection::input && port->m_direction != PinDirection::inout))
        {
            log_debug(
                "module", "port '{}' is not an input port of module '{}' with ID {} in netlist with ID {}.", port_name, this->get_name(), this->get_id(), m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return port->m_net;
    }

    Net* Module::get_output_port_net(const std::string& port_name) const
    {
        Port* port = get_port_by_name(port_name);
        if (port == nullptr || (port->m_direction != PinDirection::output && port->m_direction != PinDirection::inout))
        {
            log_debug(
                "module", "port '{}' is not an input port of module '{}' with ID {} in netlist with ID {}.", port_name, this->get_name(), this->get_id(), m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return port->m_net;
    }

    std::map<Net*, std::string> Module::get_input_port_names() const
    {
        std::map<Net*, std::string> ports;
        for (Port* port : get_ports([](Port* p) { return p->m_direction == PinDirection::input || p->m_direction == PinDirection::inout; }))
        {
            ports[port->m_net] = port->m_name;
        }

        return ports;
    }

    std::map<Net*, std::string> Module::get_output_port_names() const
    {
        std::map<Net*, std::string> ports;
        for (Port* port : get_ports([](Port* p) { return p->m_direction == PinDirection::output || p->m_direction == PinDirection::inout; }))
        {
            ports[port->m_net] = port->m_name;
        }

        return ports;
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

    void Module::create_port(const std::string& port_name, Net* port_net, PinDirection direction, PinType type) const
    {
        std::unique_ptr<Port> port_owner = std::make_unique<Port>(port_name, port_net);
        Port* port                       = port_owner.get();
        m_ports.push_back(std::move(port_owner));
        m_ports_raw.push_back(port);
        m_name_to_port[port_name] = port;
        m_net_to_port[port_net]   = port;
        port->m_direction         = direction;
        m_port_nets.insert(port_net);
    }

    void Module::remove_port(Port* port) const
    {
        m_port_nets.erase(port->m_net);
        m_ports_raw.erase(std::find(m_ports_raw.begin(), m_ports_raw.end(), port));
        m_name_to_port.erase(port->m_name);
        m_net_to_port.erase(port->m_net);
        if (!port->m_group.first.empty())
        {
            std::vector<Port*>& group_ports = m_port_groups.at(port->m_group.first);
            group_ports.erase(std::find(group_ports.begin(), group_ports.end(), port));
        }
        m_ports.erase(std::find_if(m_ports.begin(), m_ports.end(), [port](const std::unique_ptr<Port>& p) { return p.get() == port; }));
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

    void Module::update_ports() const
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
                remove_port(m_net_to_port.at(port_net));
            }

            diff_nets.clear();

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
                } while (m_name_to_port.find(port_name) != m_name_to_port.end());

                create_port(port_name, port_net, direction);
            }

            m_ports_dirty = false;
        }
    }

    bool Module::add_port(Net* port_net, const std::string& port_name, PinType type)
    {
        if (port_net == nullptr)
        {
            log_error("module", "nullptr given as net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (port_name.empty())
        {
            log_error("module",
                      "trying to assign empty port name for net '{}' with ID {} to module '{}' with ID {} in netlist with ID {}.",
                      port_net->get_name(),
                      port_net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (m_name_to_port.find(port_name) != m_name_to_port.end())
        {
            log_debug("module",
                      "port name '{}' already exists within module '{}' with ID {} in netlist with ID {}, ignoring port assignment.",
                      port_name,
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        PinDirection direction = determine_port_direction(port_net);
        if (direction == PinDirection::none)
        {
            log_debug("module",
                      "net '{}' with ID {} is not a port net of module '{}' with ID {} in netlist with ID {}, ignoring port assignment.",
                      port_net->get_name(),
                      port_net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        std::pair<Net*, std::string> port = std::make_pair(port_net, port_name);
        std::pair<Net*, std::string>* port_ptr;

        if (auto it = m_net_to_port.find(port_net); it != m_net_to_port.end())
        {
            Port* port                = it->second;
            std::string old_name      = port->m_name;
            port->m_name              = port_name;
            m_name_to_port[port_name] = port;
            m_name_to_port.erase(old_name);
        }
        else
        {
            create_port(port_name, port_net, direction, type);
        }

        m_event_handler->notify(ModuleEvent::event::port_changed, this, port_net->get_id());

        return true;
    }

    bool Module::add_ports(const std::vector<std::pair<Net*, std::string>>& ports, PinType type)
    {
        bool success = true;

        for (const auto& [port_net, port_name] : ports)
        {
            success &= add_port(port_net, port_name, type);
        }

        return success;
    }

    const std::vector<Module::Port*>& Module::get_ports() const
    {
        update_ports();

        return m_ports_raw;
    }

    std::vector<Module::Port*> Module::get_ports(const std::function<bool(Port*)>& filter) const
    {
        update_ports();

        if (!filter)
        {
            return m_ports_raw;
        }
        std::vector<Port*> res;
        for (Port* port : m_ports_raw)
        {
            if (!filter(port))
            {
                continue;
            }
            res.push_back(port);
        }

        return res;
    }

    Module::Port* Module::get_port_by_net(Net* port_net) const
    {
        update_ports();

        if (const auto it = m_net_to_port.find(port_net); it != m_net_to_port.end())
        {
            return it->second;
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

    Module::Port* Module::get_port_by_name(const std::string& port_name) const
    {
        update_ports();

        if (auto it = m_name_to_port.find(port_name); it != m_name_to_port.end())
        {
            return it->second;
        }

        log_debug("module", "'{}' is not a port of module '{}' with ID {} in netlist with ID {}.", port_name, m_name, m_id, m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    bool Module::assign_port_group(const std::string& group, const std::vector<std::pair<u32, Port*>>& port_indices)
    {
        update_ports();

        if (m_port_groups.find(group) != m_port_groups.end())
        {
            log_error("module",
                      "port group '{}' could not be added to module '{}' with ID {} in netlist with ID {} since a port group with the same name already exists.",
                      group,
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        for (const auto& port : port_indices)
        {
            if (m_name_to_port.find(port.second->m_name) == m_name_to_port.end())
            {
                log_error("module",
                          "could not assign port '{}' to group '{}' of module '{}' with ID {} in netlist with ID {}.",
                          port.second->m_name,
                          group,
                          m_name,
                          m_id,
                          m_internal_manager->m_netlist->get_id());
                return false;
            }
        }

        std::vector<Port*> ports;
        for (const auto [index, port] : port_indices)
        {
            port->m_group = std::make_pair(group, index);
            ports.push_back(port);
        }
        m_port_groups[group] = ports;

        return true;
    }

    const std::unordered_map<std::string, std::vector<Module::Port*>>& Module::get_port_groups() const
    {
        update_ports();

        return m_port_groups;
    }

    std::vector<Module::Port*> Module::get_ports_of_group(const std::string& group) const
    {
        update_ports();

        if (const auto it = m_port_groups.find(group); it != m_port_groups.end())
        {
            return it->second;
        }

        log_error("module", "port group with name '{}' does not exist.", group);
        return {};
    }
}    // namespace hal
