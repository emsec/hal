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

        for (const PinGroup<ModulePin>* pin_group : get_pin_groups())
        {
            if (const PinGroup<ModulePin>* other_pin_group = other.get_pin_group(pin_group->get_name()); other_pin_group == nullptr || *other_pin_group != *pin_group)
            {
                log_info("module", "the modules with IDs {} and {} are not equal due to an unequal pin group.", m_id, other.get_id());
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

    ssize_t Module::get_hash() const
    {
        return (uintptr_t)this;
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

        if (m_internal_manager->m_net_checks_enabled)
        {
            for (Net* net : get_nets(nullptr, true))
            {
                m_parent->check_net(net, true);
            }
        }

        m_event_handler->notify(ModuleEvent::event::submodule_removed, m_parent, m_id);

        m_parent = new_parent;

        m_parent->m_submodules_map[m_id] = this;
        m_parent->m_submodules.push_back(this);

        if (m_internal_manager->m_net_checks_enabled)
        {
            for (Net* net : get_nets(nullptr, true))
            {
                m_parent->check_net(net, true);
            }
        }

        m_event_handler->notify(ModuleEvent::event::parent_changed, this);
        m_event_handler->notify(ModuleEvent::event::submodule_added, m_parent, m_id);

        return true;
    }

    bool Module::is_parent_module_of(const Module* module, bool recursive) const
    {
        if (module == nullptr)
        {
            return false;
        }
        for (auto sm : m_submodules)
        {
            if (sm == module)
            {
                return true;
            }
            else if (recursive && sm->is_parent_module_of(module, true))
            {
                return true;
            }
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

    bool Module::is_submodule_of(const Module* module, bool recursive) const
    {
        if (module == nullptr)
        {
            return false;
        }
        if (m_parent == module)
        {
            return true;
        }
        else if (recursive && m_parent->is_submodule_of(module, true))
        {
            return true;
        }

        return false;
    }

    bool Module::contains_module(const Module* other, bool recursive) const
    {
        return is_parent_module_of(other, recursive);
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

    void Module::update_nets()
    {
        m_nets.clear();
        m_input_nets.clear();
        m_output_nets.clear();
        m_internal_nets.clear();

        std::unordered_set<Net*> net_cache;
        for (const Gate* gate : get_gates(nullptr, true))
        {
            for (Net* net : gate->get_fan_in_nets())
            {
                net_cache.insert(net);
            }
            for (Net* net : gate->get_fan_out_nets())
            {
                net_cache.insert(net);
            }
        }

        for (Net* net : net_cache)
        {
            NetConnectivity con = check_net_endpoints(net);
            if (con.has_internal_source || con.has_internal_destination)
            {
                m_nets.insert(net);

                if (con.has_internal_source && con.has_internal_destination)
                {
                    m_internal_nets.insert(net);
                }

                if (con.has_internal_source && con.has_internal_destination && con.has_external_source && con.has_external_destination)
                {
                    m_input_nets.insert(net);
                    m_output_nets.insert(net);
                }
                else if (con.has_external_source && con.has_internal_destination)
                {
                    m_input_nets.insert(net);
                }
                else if (con.has_internal_source && con.has_external_destination)
                {
                    m_output_nets.insert(net);
                }
            }
        }
    }

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

    std::unordered_set<Net*> Module::get_nets(const std::function<bool(Net*)>& filter, bool recursive) const
    {
        std::unordered_set<Net*> res;
        if (!filter)
        {
            return m_nets;
        }
        else
        {
            for (Net* n : m_nets)
            {
                if (!filter(n))
                {
                    continue;
                }
                res.insert(n);
            }
        }

        if (recursive)
        {
            for (auto sm : m_submodules)
            {
                auto more = sm->get_nets(filter, true);
                res.reserve(res.size() + more.size());
                res.insert(more.begin(), more.end());
            }
        }

        return res;
    }

    const std::unordered_set<Net*>& Module::get_input_nets() const
    {
        return m_input_nets;
    }

    const std::unordered_set<Net*>& Module::get_output_nets() const
    {
        return m_output_nets;
    }

    const std::unordered_set<Net*>& Module::get_internal_nets() const
    {
        return m_internal_nets;
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

    Module::NetConnectivity Module::check_net_endpoints(const Net* net) const
    {
        std::vector<Endpoint*> sources      = net->get_sources();
        std::vector<Endpoint*> destinations = net->get_destinations();

        NetConnectivity res;
        res.has_external_source      = net->is_global_input_net();
        res.has_internal_source      = false;
        res.has_external_destination = net->is_global_output_net();
        res.has_internal_destination = false;

        for (Endpoint* ep : sources)
        {
            if (Module* mod = ep->get_gate()->get_module(); this != mod && !is_parent_module_of(mod, true))
            {
                res.has_external_source = true;
            }
            else
            {
                res.has_internal_source = true;
            }

            if (res.has_external_source && res.has_internal_source)
            {
                break;
            }
        }

        for (Endpoint* ep : destinations)
        {
            if (Module* mod = ep->get_gate()->get_module(); this != mod && !is_parent_module_of(mod, true))
            {
                res.has_external_destination = true;
            }
            else
            {
                res.has_internal_destination = true;
            }

            if (res.has_external_destination && res.has_internal_destination)
            {
                break;
            }
        }

        return res;
    }

    void Module::check_net(Net* net, bool recursive)
    {
        NetConnectivity con = check_net_endpoints(net);
        if (con.has_internal_source && con.has_internal_destination)
        {
            m_internal_nets.insert(net);
        }
        else
        {
            m_internal_nets.erase(net);
        }

        if (con.has_internal_source || con.has_internal_destination)
        {
            m_nets.insert(net);
        }
        else
        {
            m_nets.erase(net);
        }

        if (con.has_internal_source && con.has_internal_destination && con.has_external_source && con.has_external_destination)
        {
            if (m_input_nets.find(net) == m_input_nets.end() || m_output_nets.find(net) == m_output_nets.end())
            {
                m_input_nets.insert(net);
                m_output_nets.insert(net);
                if (ModulePin* pin = get_pin(net); pin != nullptr)
                {
                    pin->m_direction = PinDirection::inout;
                    m_event_handler->notify(ModuleEvent::event::pin_changed, this);
                }
                else
                {
                    assign_pin_net(net, PinDirection::inout);
                }
            }
        }
        else
        {
            if (con.has_external_source && con.has_internal_destination)
            {
                if (m_input_nets.find(net) == m_input_nets.end())
                {
                    m_input_nets.insert(net);
                    if (ModulePin* pin = get_pin(net); pin != nullptr)
                    {
                        pin->m_direction = PinDirection::input;
                        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
                    }
                    else
                    {
                        assign_pin_net(net, PinDirection::input);
                    }
                }
            }
            else
            {
                if (m_input_nets.find(net) != m_input_nets.end())
                {
                    m_input_nets.erase(net);
                    remove_pin_net(net);
                }
            }

            if (con.has_internal_source && con.has_external_destination)
            {
                if (m_output_nets.find(net) == m_output_nets.end())
                {
                    m_output_nets.insert(net);
                    if (ModulePin* pin = get_pin(net); pin != nullptr)
                    {
                        pin->m_direction = PinDirection::output;
                        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
                    }
                    else
                    {
                        assign_pin_net(net, PinDirection::output);
                    }
                }
            }
            else
            {
                if (m_output_nets.find(net) != m_output_nets.end())
                {
                    m_output_nets.erase(net);
                    remove_pin_net(net);
                }
            }
        }

        if (m_internal_manager->m_net_checks_enabled && recursive && m_parent != nullptr)
        {
            m_parent->check_net(net, true);
        }
    }

    /*
     * ################################################################
     *      pin functions
     * ################################################################
     */

    ModulePin* Module::assign_pin(const std::string& name, Net* net, PinType type, bool create_group)
    {
        if (m_internal_manager->m_net_checks_enabled)
        {
            log_error("module",
                      "cannot manually assign pin as automatic net checks are enabled. Disable these checks using 'Netlist::enable_automatic_net_checks(false)' in case you want to manage pins "
                      "manually.");
            return nullptr;
        }

        if (name.empty())
        {
            log_error("module",
                      "empty name provided for pin at net '{}' with ID {} within module '{}' with ID {} in netlist with ID {}.",
                      net->get_name(),
                      net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        if (net == nullptr)
        {
            log_error("module", "provided 'nullptr' for net of pin '{}' within module '{}' with ID {} in netlist with ID {}.", name, m_name, m_id, m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        PinDirection direction;
        bool is_input  = is_input_net(net);
        bool is_output = is_output_net(net);

        if (is_input && is_output)
        {
            direction = PinDirection::inout;
        }
        else if (is_input)
        {
            direction = PinDirection::input;
        }
        else if (is_output)
        {
            direction = PinDirection::output;
        }
        else
        {
            log_error("module",
                      "cannot assign pin '{}' to net '{}' with ID {} as net is not an input or output of module '{}' with ID {} in netlist with ID {}.",
                      name,
                      net->get_name(),
                      net->get_id(),
                      m_name,
                      m_id,
                      m_internal_manager->m_netlist->get_id());
            return nullptr;
        }

        return assign_pin_net(net, direction, name, type, create_group);
    }

    std::vector<ModulePin*> Module::get_pins(const std::function<bool(ModulePin*)>& filter) const
    {
        std::vector<ModulePin*> res;
        if (!filter)
        {
            res.reserve(m_pins.size());
            for (const auto& group : m_pin_groups)
            {
                std::vector<ModulePin*> pins = group->get_pins();
                res.insert(res.end(), pins.begin(), pins.end());
            }
        }
        else
        {
            for (PinGroup<ModulePin>* group : m_pin_groups_ordered)
            {
                for (ModulePin* pin : group->get_pins())
                {
                    if (filter(pin))
                    {
                        res.push_back(pin);
                    }
                }
            }
        }
        return res;
    }

    std::vector<PinGroup<ModulePin>*> Module::get_pin_groups(const std::function<bool(PinGroup<ModulePin>*)>& filter) const
    {
        std::vector<PinGroup<ModulePin>*> res;
        if (!filter)
        {
            res.reserve(m_pin_groups_ordered.size());
            res.insert(res.end(), m_pin_groups_ordered.begin(), m_pin_groups_ordered.end());
        }
        else
        {
            for (PinGroup<ModulePin>* group : m_pin_groups_ordered)
            {
                if (filter(group))
                {
                    res.push_back(group);
                }
            }
        }
        return res;
    }

    ModulePin* Module::get_pin(const std::string& name) const
    {
        if (name.empty())
        {
            return nullptr;
        }

        if (const auto it = m_pin_names_map.find(name); it != m_pin_names_map.end())
        {
            return it->second;
        }

        return nullptr;
    }

    ModulePin* Module::get_pin(Net* net) const
    {
        if (net == nullptr)
        {
            return nullptr;
        }

        if (const auto it = std::find_if(m_pins.begin(), m_pins.end(), [net](const std::unique_ptr<ModulePin>& pin) { return pin->get_net() == net; }); it != m_pins.end())
        {
            return it->get();
        }

        return nullptr;
    }

    PinGroup<ModulePin>* Module::get_pin_group(const std::string& name) const
    {
        if (name.empty())
        {
            return nullptr;
        }

        if (const auto it = m_pin_group_names_map.find(name); it != m_pin_group_names_map.end())
        {
            return it->second;
        }

        return nullptr;
    }

    bool Module::set_pin_name(ModulePin* pin, const std::string& new_name)
    {
        if (pin == nullptr || new_name.empty())
        {
            return false;
        }

        if (const auto it = m_pin_names_map.find(pin->m_name); it == m_pin_names_map.end() || it->second != pin)
        {
            // pin does not belong to current module
            return false;
        }

        if (m_pin_names_map.find(new_name) != m_pin_names_map.end())
        {
            // pin names must be unique
            return false;
        }

        std::string old_name = pin->m_name;
        pin->m_name          = new_name;
        m_pin_names_map.erase(old_name);
        m_pin_names_map[new_name] = pin;
        if (PinGroup<ModulePin>* group = pin->m_group.first; group != nullptr)
        {
            group->m_pin_name_map.erase(old_name);
            group->m_pin_name_map[new_name] = pin;
        }

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);

        return true;
    }

    bool Module::set_pin_group_name(PinGroup<ModulePin>* pin_group, const std::string& new_name)
    {
        if (pin_group == nullptr || new_name.empty())
        {
            return false;
        }

        if (const auto it = m_pin_group_names_map.find(pin_group->m_name); it == m_pin_group_names_map.end() || it->second != pin_group)
        {
            // pin group does not belong to current module
            return false;
        }

        if (m_pin_group_names_map.find(new_name) != m_pin_group_names_map.end())
        {
            // pin groups must be unique
            return false;
        }

        std::string old_name = pin_group->m_name;
        pin_group->m_name    = new_name;
        m_pin_group_names_map.erase(old_name);
        m_pin_group_names_map[new_name] = pin_group;

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);

        return true;
    }

    bool Module::set_pin_type(ModulePin* pin, PinType new_type)
    {
        if (pin == nullptr)
        {
            return false;
        }

        if (const auto it = m_pin_names_map.find(pin->m_name); it == m_pin_names_map.end() || it->second != pin)
        {
            // pin does not belong to current module
            return false;
        }

        if (pin->m_group.first->size() != 1)
        {
            return false;
        }

        if (pin->m_type != new_type)
        {
            pin->m_type                = new_type;
            pin->m_group.first->m_type = new_type;
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }
        return true;
    }

    bool Module::set_pin_group_type(PinGroup<ModulePin>* pin_group, PinType new_type)
    {
        if (pin_group == nullptr)
        {
            return false;
        }

        if (const auto it = m_pin_group_names_map.find(pin_group->m_name); it == m_pin_group_names_map.end() || it->second != pin_group)
        {
            // pin group does not belong to current module
            return false;
        }

        if (pin_group->m_type != new_type)
        {
            pin_group->m_type = new_type;
            for (const std::unique_ptr<ModulePin>& pin : m_pins)
            {
                pin->m_type = new_type;
            }
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }
        return true;
    }

    PinGroup<ModulePin>* Module::create_pin_group(const std::string& name, const std::vector<ModulePin*> pins, bool ascending, u32 start_index)
    {
        if (name.empty() || pins.empty())
        {
            return nullptr;
        }

        if (m_pin_group_names_map.find(name) != m_pin_group_names_map.end())
        {
            // pin group names must be unique
            return nullptr;
        }

        PinDirection master_direction = pins.front()->get_direction();
        PinType master_type           = pins.front()->get_type();
        for (ModulePin* pin : pins)
        {
            if (pin == nullptr)
            {
                return nullptr;
            }

            if (pin->get_direction() != master_direction || pin->get_type() != master_type)
            {
                // wrong direction or type
                return nullptr;
            }

            if (const auto it = m_pin_names_map.find(pin->m_name); it == m_pin_names_map.end() || it->second != pin)
            {
                // pin does not belong to module
                return nullptr;
            }
        }

        std::unique_ptr<PinGroup<ModulePin>> pin_group_owner(new PinGroup<ModulePin>(name, master_direction, master_type, ascending, start_index));
        PinGroup<ModulePin>* pin_group = pin_group_owner.get();
        m_pin_groups.push_back(std::move(pin_group_owner));
        m_pin_groups_ordered.push_back(pin_group);
        m_pin_group_names_map[name] = pin_group;

        bool failed = false;
        for (ModulePin* pin : pins)
        {
            if (!assign_pin_to_group(pin_group, pin))
            {
                failed = true;
                break;
            }
        }

        if (failed)
        {
            delete_pin_group(pin_group);
            return nullptr;
        }
        m_event_handler->notify(ModuleEvent::event::pin_changed, this);

        return pin_group;
    }

    bool Module::delete_pin_group(PinGroup<ModulePin>* pin_group)
    {
        if (pin_group == nullptr)
        {
            return false;
        }

        if (const auto it = m_pin_group_names_map.find(pin_group->m_name); it == m_pin_group_names_map.end() || it->second != pin_group)
        {
            // pin group does not belong to current module
            return false;
        }

        bool removed_pins = false;

        std::vector<ModulePin*> pins_copy(pin_group->m_pins.begin(), pin_group->m_pins.end());
        for (ModulePin* pin : pins_copy)
        {
            removed_pins     = true;
            std::string name = pin->m_name;
            while (m_pin_group_names_map.find(name) != m_pin_group_names_map.end())
            {
                name += "_";
            }
            create_pin_group(name, {pin});
        }

        if (removed_pins)
        {
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }
        return true;
    }

    bool Module::assign_pin_to_group(PinGroup<ModulePin>* pin_group, ModulePin* pin)
    {
        if (pin_group == nullptr || pin == nullptr)
        {
            return false;
        }

        if (const auto it = m_pin_group_names_map.find(pin_group->m_name); it == m_pin_group_names_map.end() || it->second != pin_group)
        {
            // pin group does not belong to module
            return false;
        }

        if (const auto it = m_pin_names_map.find(pin->m_name); it == m_pin_names_map.end() || it->second != pin)
        {
            // pin does not belong to module
            return false;
        }

        if (pin->get_direction() != pin_group->get_direction() || pin->get_type() != pin_group->get_type())
        {
            // wrong direction or type
            return false;
        }

        if (PinGroup<ModulePin>* pg = pin->m_group.first; pg != nullptr)
        {
            // remove from old group and potentially delete old group if empty
            pg->remove_pin(pin);
            if (pg->empty())
            {
                m_pin_group_names_map.erase(pg->m_name);
                m_pin_groups_ordered.erase(std::find(m_pin_groups_ordered.begin(), m_pin_groups_ordered.end(), pg));
                m_pin_groups.erase(std::find_if(m_pin_groups.begin(), m_pin_groups.end(), [pg](const std::unique_ptr<PinGroup<ModulePin>>& pg2) { return pg2.get() == pg; }));
            }
        }

        if (!pin_group->assign_pin(pin))
        {
            return false;
        }

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        return true;
    }

    bool Module::move_pin_within_group(PinGroup<ModulePin>* pin_group, ModulePin* pin, u32 new_index)
    {
        if (pin_group == nullptr || pin == nullptr)
        {
            return false;
        }

        if (pin->m_group.first != pin_group)
        {
            // pin does not belong to group
            return false;
        }

        if (const auto it = m_pin_group_names_map.find(pin_group->m_name); it == m_pin_group_names_map.end() || it->second != pin_group)
        {
            // pin group does not belong to module
            return false;
        }

        if (const auto it = m_pin_names_map.find(pin->m_name); it == m_pin_names_map.end() || it->second != pin)
        {
            // pin does not belong to module
            return false;
        }

        if (pin_group->move_pin(pin, new_index))
        {
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
            return true;
        }
        return false;
    }

    bool Module::remove_pin_from_group(PinGroup<ModulePin>* pin_group, ModulePin* pin)
    {
        if (pin_group == nullptr || pin == nullptr)
        {
            return false;
        }

        if (pin->m_group.first != pin_group)
        {
            // pin does not belong to group
            return false;
        }

        if (const auto it = m_pin_group_names_map.find(pin_group->m_name); it == m_pin_group_names_map.end() || it->second != pin_group)
        {
            // pin group does not belong to module
            return false;
        }

        if (const auto it = m_pin_names_map.find(pin->m_name); it == m_pin_names_map.end() || it->second != pin)
        {
            // pin does not belong to module
            return false;
        }

        std::string name = pin->m_name;
        while (m_pin_group_names_map.find(name) != m_pin_group_names_map.end())
        {
            name += "_";
        }

        if (create_pin_group(name, {pin}) != nullptr)
        {
            return true;
        }
        return false;
    }

    ModulePin* Module::assign_pin_net(Net* net, PinDirection direction, const std::string& name, PinType type, bool create_group)
    {
        std::string name_internal;
        if (m_pin_names_map.find(name) == m_pin_names_map.end() && m_pin_group_names_map.find(name) != m_pin_group_names_map.end())
        {
            log_warning("module",
                        "assigning auto generated pin name to net '{}' with ID {} as '{}' is already in use within module '{}' with ID {} in netlist with ID {}.",
                        net->get_name(),
                        net->get_id(),
                        name,
                        m_name,
                        m_id,
                        m_internal_manager->m_netlist->get_id());
        }
        else
        {
            name_internal = name;
        }

        if (name_internal.empty())
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

            do
            {
                name_internal = port_prefix + "(" + std::to_string((*index_counter)++) + ")";
            } while (m_pin_names_map.find(name_internal) != m_pin_names_map.end() && m_pin_group_names_map.find(name_internal) != m_pin_group_names_map.end());
        }

        // create pin
        std::unique_ptr<ModulePin> pin_owner(new ModulePin(name_internal, net, direction, type));
        ModulePin* pin = pin_owner.get();
        m_pins.push_back(std::move(pin_owner));
        m_pin_names_map[name_internal] = pin;

        if (create_group)
        {
            // create pin group (every pin must be within exactly one pin group)
            std::unique_ptr<PinGroup<ModulePin>> pin_group_owner(new PinGroup<ModulePin>(name_internal, pin->m_direction, pin->m_type));
            PinGroup<ModulePin>* pin_group = pin_group_owner.get();
            m_pin_groups.push_back(std::move(pin_group_owner));
            m_pin_groups_ordered.push_back(pin_group);
            m_pin_group_names_map[name_internal] = pin_group;

            assign_pin_to_group(pin_group, pin);    // triggers 'pin_changed' event
        }
        else
        {
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }

        return pin;
    }

    bool Module::remove_pin_net(Net* net)
    {
        ModulePin* pin = get_pin(net);
        if (pin == nullptr)
        {
            return false;
        }

        if (PinGroup<ModulePin>* pin_group = pin->m_group.first; pin_group != nullptr)
        {
            pin_group->remove_pin(pin);
            if (pin_group->empty())
            {
                // remove pin group
                m_pin_group_names_map.erase(pin_group->m_name);
                m_pin_groups_ordered.erase(std::find(m_pin_groups_ordered.begin(), m_pin_groups_ordered.end(), pin_group));
                m_pin_groups.erase(std::find_if(m_pin_groups.begin(), m_pin_groups.end(), [pin_group](const std::unique_ptr<PinGroup<ModulePin>>& pg) { return pg.get() == pin_group; }));
            }
        }

        m_pin_names_map.erase(pin->m_name);
        m_pins.erase(std::find_if(m_pins.begin(), m_pins.end(), [pin](const std::unique_ptr<ModulePin>& p) { return p.get() == pin; }));

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);

        return true;
    }
}    // namespace hal
