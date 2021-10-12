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
        if (input_net == nullptr)
        {
            log_error("module", "nullptr given as input net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (port_name.empty())
        {
            log_error("module",
                      "trying to assign empty input port name for net '{}' with ID {} to module '{}' with ID {} in netlist with ID {}.",
                      input_net->get_name(),
                      input_net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (m_input_port_names.find(port_name) != m_input_port_names.end())
        {
            log_debug("module",
                      "input port name '{}' already exists within module '{}' with ID {} in netlist with ID {}, ignoring port assignment.",
                      port_name,
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        const std::vector<Net*>& input_nets = get_input_nets();
        if (auto it = std::find(input_nets.begin(), input_nets.end(), input_net); it == input_nets.end())
        {
            log_debug("module",
                      "net '{}' with ID {} is not an input net of module '{}' with ID {} in netlist with ID {}, ignoring port assignment.",
                      input_net->get_name(),
                      input_net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        m_named_input_nets.insert(input_net);
        m_input_port_names.insert(port_name);

        if (auto prev_name = m_input_net_to_port_name.find(input_net); prev_name != m_input_net_to_port_name.end())
        {
            m_input_port_names.erase(prev_name->second);
        }
        m_input_net_to_port_name[input_net] = port_name;

        m_event_handler->notify(ModuleEvent::event::port_changed, this, input_net->get_id());

        return true;
    }

    bool Module::set_output_port_name(Net* output_net, const std::string& port_name)
    {
        if (output_net == nullptr)
        {
            log_error("module", "nullptr given as output net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (port_name.empty())
        {
            log_error("module",
                      "trying to assign empty output port name for net '{}' with ID {} to module '{}' with ID {} in netlist with ID {}.",
                      output_net->get_name(),
                      output_net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        if (m_output_port_names.find(port_name) != m_output_port_names.end())
        {
            log_debug("module",
                      "output port name '{}' already exists within module '{}' with ID {} in netlist with ID {}, ignoring port assignment.",
                      port_name,
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        const std::vector<Net*>& output_nets = get_output_nets();
        if (auto it = std::find(output_nets.begin(), output_nets.end(), output_net); it == output_nets.end())
        {
            log_debug("module",
                      "net '{}' with ID {} is not an output net of module '{}' with ID {} in netlist with ID {}, ignoring port assignment.",
                      output_net->get_name(),
                      output_net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        m_named_output_nets.insert(output_net);
        m_output_port_names.insert(port_name);
        if (auto prev_name = m_output_net_to_port_name.find(output_net); prev_name != m_output_net_to_port_name.end())
        {
            m_output_port_names.erase(prev_name->second);
        }
        m_output_net_to_port_name[output_net] = port_name;

        m_event_handler->notify(ModuleEvent::event::port_changed, this, output_net->get_id());

        return true;
    }

    std::string Module::get_input_port_name(Net* net) const
    {
        if (net == nullptr)
        {
            log_error("module", "nullptr given as input net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return "";
        }

        const std::vector<Net*>& input_nets = get_input_nets();
        if (auto it = std::find(input_nets.begin(), input_nets.end(), net); it == input_nets.end())
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

        std::string port_name;
        if (auto it = m_input_net_to_port_name.find(net); it != m_input_net_to_port_name.end())
        {
            port_name = it->second;
        }
        else
        {
            do
            {
                port_name = "I(" + std::to_string(m_next_input_port_id++) + ")";
            } while (m_input_port_names.find(port_name) != m_input_port_names.end());

            m_named_input_nets.insert(net);
            m_input_port_names.insert(port_name);
            m_input_net_to_port_name.emplace(net, port_name);
        }

        return port_name;
    }

    std::string Module::get_output_port_name(Net* net) const
    {
        if (net == nullptr)
        {
            log_error("module", "nullptr given as output net for module '{}' with ID {} in netlist with ID {}.", m_name, m_id, m_internal_manager->m_netlist->get_id());
            return "";
        }

        const std::vector<Net*>& output_nets = get_output_nets();
        if (auto it = std::find(output_nets.begin(), output_nets.end(), net); it == output_nets.end())
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

        std::string port_name;
        if (auto it = m_output_net_to_port_name.find(net); it != m_output_net_to_port_name.end())
        {
            port_name = it->second;
        }
        else
        {
            do
            {
                port_name = "O(" + std::to_string(m_next_output_port_id++) + ")";
            } while (m_output_port_names.find(port_name) != m_output_port_names.end());

            m_named_output_nets.insert(net);
            m_output_port_names.insert(port_name);
            m_output_net_to_port_name.emplace(net, port_name);
        }

        return port_name;
    }

    Net* Module::get_input_port_net(const std::string& port_name) const
    {
        for (const auto& [net, name] : m_input_net_to_port_name)
        {
            if (name == port_name)
            {
                return net;
            }
        }

        log_debug("module", "port '{}' is not an input port of module '{}' with ID {} in netlist with ID {}.", port_name, this->get_name(), this->get_id(), m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    Net* Module::get_output_port_net(const std::string& port_name) const
    {
        for (const auto& [net, name] : m_output_net_to_port_name)
        {
            if (name == port_name)
            {
                return net;
            }
        }

        log_debug("module", "port '{}' is not an output port of module '{}' with ID {} in netlist with ID {}.", port_name, this->get_name(), this->get_id(), m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    const std::map<Net*, std::string>& Module::get_input_port_names() const
    {
        const std::vector<Net*>& input_nets = get_input_nets();

        std::vector<Net*> diff;

        // find nets that are still in the port map but no longer an input net
        std::set_difference(m_named_input_nets.begin(), m_named_input_nets.end(), input_nets.begin(), input_nets.end(), std::back_inserter(diff));
        for (auto net : diff)
        {
            m_named_input_nets.erase(net);
            m_input_port_names.erase(m_input_net_to_port_name.at(net));
            m_input_net_to_port_name.erase(net);
        }

        diff.clear();

        // find nets that are input nets but have not yet been assigned a port name
        std::set_difference(input_nets.begin(), input_nets.end(), m_named_input_nets.begin(), m_named_input_nets.end(), std::back_inserter(diff));
        for (auto net : diff)
        {
            std::string port_name;
            do
            {
                port_name = "I(" + std::to_string(m_next_input_port_id++) + ")";
            } while (m_input_port_names.find(port_name) != m_input_port_names.end());

            m_named_input_nets.insert(net);
            m_input_port_names.insert(port_name);
            m_input_net_to_port_name.emplace(net, port_name);
        }

        return m_input_net_to_port_name;
    }

    const std::map<Net*, std::string>& Module::get_output_port_names() const
    {
        const std::vector<Net*>& output_nets = get_output_nets();
        std::set<Net*> diff;

        // find nets that are still in the port map but no longer an output net
        std::set_difference(m_named_output_nets.begin(), m_named_output_nets.end(), output_nets.begin(), output_nets.end(), std::inserter(diff, diff.begin()));
        for (auto net : diff)
        {
            m_named_output_nets.erase(net);
            m_output_port_names.erase(m_output_net_to_port_name.at(net));
            m_output_net_to_port_name.erase(net);
        }

        diff.clear();

        // find nets that are output nets but have not yet been assigned a port name
        std::set_difference(output_nets.begin(), output_nets.end(), m_named_output_nets.begin(), m_named_output_nets.end(), std::inserter(diff, diff.begin()));
        for (auto net : diff)
        {
            std::string port_name;
            do
            {
                port_name = "O(" + std::to_string(m_next_output_port_id++) + ")";
            } while (m_output_port_names.find(port_name) != m_output_port_names.end());

            m_named_output_nets.insert(net);
            m_output_port_names.insert(port_name);
            m_output_net_to_port_name.emplace(net, port_name);
        }

        return m_output_net_to_port_name;
    }

    void Module::set_next_input_port_id(u32 id)
    {
        m_next_input_port_id = id;
    }

    u32 Module::get_next_input_port_id() const
    {
        return m_next_input_port_id;
    }

    void Module::set_next_output_port_id(u32 id)
    {
        m_next_output_port_id = id;
    }

    u32 Module::get_next_output_port_id() const
    {
        return m_next_output_port_id;
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

    bool Module::change_port_name(std::pair<Net*, std::string>* port_ptr, const std::string& new_name, PinDirection new_direction) const
    {
        Net* port_net           = port_ptr->first;
        std::string old_name    = port_ptr->second;
        *port_ptr               = std::make_pair(port_net, new_name);
        m_net_to_port[port_net] = port_ptr;
        m_name_to_port.erase(old_name);
        m_name_to_port[new_name] = port_ptr;

        PinDirection old_direction         = m_port_name_to_direction.at(old_name);
        m_port_name_to_direction[new_name] = new_direction;
        m_port_name_to_direction.erase(old_name);
        m_port_direction_to_names.at(old_direction).erase(old_name);
        m_port_direction_to_names[new_direction].insert(new_name);

        PinType type                  = m_port_name_to_type.at(old_name);
        m_port_name_to_type[new_name] = type;
        m_port_name_to_type.erase(old_name);
        m_port_type_to_names.at(type).erase(old_name);
        m_port_type_to_names[type].insert(new_name);

        std::string group              = m_port_name_to_group.at(old_name);
        m_port_name_to_group[new_name] = group;
        m_port_name_to_group.erase(old_name);
        m_port_groups;    // TODO here
        m_port_group_indices;
    }

    std::pair<Net*, std::string> Module::create_port(Net* port_net, const std::string& port_name, PinDirection direction, PinType type) const
    {
        m_ports.push_back(std::make_pair(port_net, port_name));
        std::pair<Net*, std::string>* port_ptr = &(m_ports.back());
        m_net_to_port[port_net]                = port_ptr;
        m_name_to_port[port_name]              = port_ptr;

        m_port_name_to_direction[port_name] = direction;
        m_port_direction_to_names[direction].insert(port_name);

        m_port_name_to_type[port_name] = type;
        m_port_type_to_names[type].insert(port_name);
    }

    void Module::update_ports() const
    {
        // TODO
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
            // change existing port
            change_port_name(it->second, port_name, direction);
        }
        else
        {
            create_port(port_net, port_name, direction, type);
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

    const std::vector<std::pair<Net*, std::string>>& Module::get_ports() const
    {
        update_ports();

        return m_ports;
    }

    std::string Module::get_port_name(Net* port_net) const
    {
        update_ports();

        if (const auto it = m_net_to_port.find(port_net); it != m_net_to_port.end())
        {
            return it->second->second;
        }

        log_debug("module",
                  "net '{}' with ID {} is not connected to a port of module '{}' with ID {} in netlist with ID {}.",
                  port_net->get_name(),
                  port_net->get_id(),
                  m_name,
                  m_id,
                  m_internal_manager->m_netlist->get_id());
        return "";
    }

    Net* Module::get_port_net(const std::string& port_name) const
    {
        update_ports();

        if (auto it = m_name_to_port.find(port_name); it != m_name_to_port.end())
        {
            return it->second->first;
        }

        log_debug("module", "'{}' is not a port of module '{}' with ID {} in netlist with ID {}.", port_name, m_name, m_id, m_internal_manager->m_netlist->get_id());
        return nullptr;
    }

    PinDirection Module::get_port_direction(const std::string& port_name) const
    {
        update_ports();

        if (const auto it = m_port_name_to_direction.find(port_name); it != m_port_name_to_direction.end())
        {
            return it->second;
        }

        return PinDirection::none;
    }

    const std::unordered_map<std::string, PinDirection>& Module::get_port_directions() const
    {
        update_ports();

        return m_port_name_to_direction;
    }

    std::unordered_set<std::string> Module::get_ports_of_direction(PinDirection direction) const
    {
        update_ports();

        if (const auto it = m_port_direction_to_names.find(direction); it != m_port_direction_to_names.end())
        {
            return it->second;
        }

        return {};
    }

    bool Module::assign_port_type(const std::string& port_name, PinType type)
    {
        update_ports();

        if (m_name_to_port.find(port_name) != m_name_to_port.end())
        {
            if (const auto it = m_port_name_to_type.find(port_name); it != m_port_name_to_type.end())
            {
                m_port_type_to_names.at(it->second).erase(port_name);
            }
            m_port_name_to_type[port_name] = type;
            m_port_type_to_names[type].insert(port_name);
            return true;
        }

        log_error("module",
                  "could not assign type '{}' to port '{}' of module '{}' with ID {} in netlist with ID {}.",
                  enum_to_string<PinType>(type),
                  port_name,
                  m_name,
                  m_id,
                  m_internal_manager->m_netlist->get_id());
        return false;
    }

    PinType Module::get_port_type(const std::string& port_name) const
    {
        update_ports();

        if (const auto it = m_port_name_to_type.find(port_name); it != m_port_name_to_type.end())
        {
            return it->second;
        }

        return PinType::none;
    }

    const std::unordered_map<std::string, PinType>& Module::get_port_types() const
    {
        update_ports();

        return m_port_name_to_type;
    }

    std::unordered_set<std::string> Module::get_ports_of_type(PinType type) const
    {
        update_ports();

        if (const auto it = m_port_type_to_names.find(type); it != m_port_type_to_names.end())
        {
            return it->second;
        }

        return {};
    }

    bool Module::assign_port_group(const std::string& group, const std::vector<std::pair<u32, std::string>>& ports)
    {
        update_ports();

        if (m_port_groups.find(group) != m_port_groups.end())
        {
            log_error("module",
                      "port group '{}' could not be added to module '{}' with ID {} in netlist with ID {} since a port group with the same name does already exists.",
                      group,
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return false;
        }

        for (const auto& port : ports)
        {
            if (m_name_to_port.find(port.second) == m_name_to_port.end())
            {
                log_error(
                    "module", "could not assign port '{}' to group '{}' of module '{}' with ID {} in netlist with ID {}.", port.second, group, m_name, m_id, m_internal_manager->m_netlist->get_id());
                return false;
            }
        }

        m_port_groups[group] = ports;

        std::unordered_map<u32, std::string> index_to_port_name;
        for (const auto& [index, port_name] : ports)
        {
            index_to_port_name[index]       = port_name;
            m_port_name_to_group[port_name] = group;
        }

        m_port_group_indices[group] = index_to_port_name;
        return true;
    }

    std::string Module::get_port_group(const std::string& port_name) const
    {
        update_ports();

        if (const auto it = m_port_name_to_group.find(port_name); it != m_port_name_to_group.end())
        {
            return it->second;
        }

        return "";
    }

    const std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>>& Module::get_port_groups() const
    {
        update_ports();

        return m_port_groups;
    }

    std::vector<std::pair<u32, std::string>> Module::get_ports_of_group(const std::string& group) const
    {
        update_ports();

        if (const auto it = m_port_groups.find(group); it != m_port_groups.end())
        {
            return it->second;
        }

        log_error("module", "port group with name '{}' does not exist.", group);
        return {};
    }

    std::string Module::get_port_of_group_at_index(const std::string& group, const u32 index) const
    {
        update_ports();

        if (const auto group_it = m_port_group_indices.find(group); group_it != m_port_group_indices.end())
        {
            const std::unordered_map<u32, std::string>& index_to_port = group_it->second;
            if (const auto index_it = index_to_port.find(index); index_it != index_to_port.end())
            {
                return index_it->second;
            }

            log_error("module", "port group with name '{}' does not have a port with index {}.", group, index);
            return "";
        }

        log_error("module", "port group with name '{}' does not exist.", group);
        return "";
    }
}    // namespace hal
