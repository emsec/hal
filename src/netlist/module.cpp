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

        m_next_pin_id       = 1;
        m_next_pin_group_id = 1;

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
            if (const auto other_pin_group_res = other.get_pin_group_by_id(pin_group->get_id()); other_pin_group_res.is_error() || *(other_pin_group_res.get()) != *pin_group)
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
            for (Net* net : m_parent->get_nets(nullptr, true))
            {
                if (auto res = m_parent->check_net(net, true); res.is_error())
                {
                    log_error("module", "{}", res.get_error().get());
                }
            }
        }

        m_event_handler->notify(ModuleEvent::event::submodule_removed, m_parent, m_id);

        m_parent = new_parent;

        m_parent->m_submodules_map[m_id] = this;
        m_parent->m_submodules.push_back(this);

        if (m_internal_manager->m_net_checks_enabled)
        {
            for (Net* net : m_parent->get_nets(nullptr, true))
            {
                if (auto res = m_parent->check_net(net, true); res.is_error())
                {
                    log_error("module", "{}", res.get_error().get());
                }
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

    Result<std::monostate> Module::check_net(Net* net, bool recursive)
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
                if (auto pin_res = get_pin_by_net(net); pin_res.is_ok())
                {
                    pin_res.get()->set_direction(PinDirection::inout);
                    m_event_handler->notify(ModuleEvent::event::pin_changed, this);
                }
                else
                {
                    if (auto res = assign_pin_net(get_unique_pin_id(), net, PinDirection::inout); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
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
                    if (auto pin_res = get_pin_by_net(net); pin_res.is_ok())
                    {
                        pin_res.get()->set_direction(PinDirection::input);
                        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
                    }
                    else
                    {
                        if (auto res = assign_pin_net(get_unique_pin_id(), net, PinDirection::input); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }
                }
            }
            else
            {
                if (m_input_nets.find(net) != m_input_nets.end())
                {
                    m_input_nets.erase(net);
                    if (auto res = remove_pin_net(net); res.is_error())
                    {
                        return res;
                    }
                }
            }

            if (con.has_internal_source && con.has_external_destination)
            {
                if (m_output_nets.find(net) == m_output_nets.end())
                {
                    m_output_nets.insert(net);
                    if (auto pin_res = get_pin_by_net(net); pin_res.is_ok())
                    {
                        pin_res.get()->set_direction(PinDirection::output);
                        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
                    }
                    else
                    {
                        if (auto res = assign_pin_net(get_unique_pin_id(), net, PinDirection::output); res.is_error())
                        {
                            return ERR(res.get_error());
                        }
                    }
                }
            }
            else
            {
                if (m_output_nets.find(net) != m_output_nets.end())
                {
                    m_output_nets.erase(net);
                    if (auto res = remove_pin_net(net); res.is_error())
                    {
                        return res;
                    }
                }
            }
        }

        if (m_internal_manager->m_net_checks_enabled && recursive && m_parent != nullptr)
        {
            if (auto res = m_parent->check_net(net, true); res.is_error())
            {
                return res;
            }
        }

        return OK({});
    }

    /*
     * ################################################################
     *      pin functions
     * ################################################################
     */

    u32 Module::get_unique_pin_id()
    {
        if (!m_free_pin_ids.empty())
        {
            return *(m_free_pin_ids.begin());
        }
        while (m_used_pin_ids.find(m_next_pin_id) != m_used_pin_ids.end())
        {
            m_next_pin_id++;
        }
        return m_next_pin_id;
    }

    u32 Module::get_unique_pin_group_id()
    {
        if (!m_free_pin_group_ids.empty())
        {
            return *(m_free_pin_group_ids.begin());
        }
        while (m_used_pin_group_ids.find(m_next_pin_group_id) != m_used_pin_group_ids.end())
        {
            m_next_pin_group_id++;
        }
        return m_next_pin_group_id;
    }

    Result<ModulePin*> Module::create_pin(const u32 id, const std::string& name, Net* net, PinType type, bool create_group)
    {
        if (m_internal_manager->m_net_checks_enabled)
        {
            return ERR(
                "cannot manually assign pin as automatic net checks are enabled. Disable these checks using 'Netlist::enable_automatic_net_checks(false)' in case you want to manage pins manually.");
        }

        if (net == nullptr)
        {
            return ERR("'nullptr' given instead of a net when trying to assign pin '" + name + "' to a net of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (name.empty())
        {
            return ERR("empty string passed as name for pin at net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + " of module '" + m_name + "' with ID " + std::to_string(m_id));
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
            return ERR("net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + " is neither an output nor an input of module '" + m_name + "' with ID " + std::to_string(m_id)
                       + ", could not assign pin with name '" + name + "'");
        }

        return assign_pin_net(id, net, direction, name, type, create_group);
    }

    Result<ModulePin*> Module::create_pin(const std::string& name, Net* net, PinType type, bool create_group)
    {
        return create_pin(get_unique_pin_id(), name, net, type, create_group);
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

    Result<ModulePin*> Module::get_pin_by_id(const u32 id) const
    {
        if (id == 0)
        {
            return ERR("pin ID 0 is invalid");
        }

        if (const auto it = m_pins_map.find(id); it != m_pins_map.end())
        {
            return OK(it->second);
        }

        return ERR("module '" + m_name + "' with ID " + std::to_string(m_id) + " contains no pin with ID " + std::to_string(id));
    }

    Result<ModulePin*> Module::get_pin_by_net(Net* net) const
    {
        if (net == nullptr)
        {
            return ERR("'nullptr' given instead of a net when trying to get a pin by net from module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = std::find_if(m_pins.begin(), m_pins.end(), [net](const std::unique_ptr<ModulePin>& pin) { return pin->get_net() == net; }); it != m_pins.end())
        {
            return OK(it->get());
        }

        return ERR("net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + " does not pass through a pin of module '" + m_name + "' with ID " + std::to_string(m_id));
    }

    Result<PinGroup<ModulePin>*> Module::get_pin_group_by_id(const u32 id) const
    {
        if (id == 0)
        {
            return ERR("provided invalid pin group ID of 0 when trying to get a pin by ID from module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(id); it != m_pin_groups_map.end())
        {
            return OK(it->second);
        }

        return ERR("module '" + m_name + "' with ID " + std::to_string(m_id) + " contains no pin group with ID " + std::to_string(id));
    }

    Result<std::monostate> Module::set_pin_name(ModulePin* pin, const std::string& new_name)
    {
        if (pin == nullptr)
        {
            return ERR("'nullptr' given instead of a pin when trying to set the name of a pin for module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (new_name.empty())
        {
            return ERR("empty string passed as new name for pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
        {
            return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin->get_name() != new_name)
        {
            pin->set_name(new_name);
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }

        return OK({});
    }

    Result<std::monostate> Module::set_pin_group_name(PinGroup<ModulePin>* pin_group, const std::string& new_name)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to set the name of a pin group for module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (new_name.empty())
        {
            return ERR("empty string passed as new name for pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " of module '" + m_name + "' with ID "
                       + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin_group->get_name() != new_name)
        {
            pin_group->set_name(new_name);
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }

        return OK({});
    }

    Result<std::monostate> Module::set_pin_type(ModulePin* pin, PinType new_type)
    {
        if (pin == nullptr)
        {
            return ERR("'nullptr' given instead of a pin when trying to set the type of a pin for module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
        {
            return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin->get_type() != new_type)
        {
            pin->set_type(new_type);
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }

        return OK({});
    }

    Result<std::monostate> Module::set_pin_group_type(PinGroup<ModulePin>* pin_group, PinType new_type)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to set the type of a pin group for module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin_group->get_type() != new_type)
        {
            pin_group->set_type(new_type);
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }
        return OK({});
    }

    Result<std::monostate> Module::set_pin_group_direction(PinGroup<ModulePin>* pin_group, PinDirection new_direction)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to set the direction of a pin group for module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin_group->get_direction() != new_direction)
        {
            pin_group->set_direction(new_direction);
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }
        return OK({});
    }

    Result<PinGroup<ModulePin>*> Module::create_pin_group(const u32 id,
                                                          const std::string& name,
                                                          const std::vector<ModulePin*> pins,
                                                          PinDirection direction,
                                                          PinType type,
                                                          bool ascending,
                                                          u32 start_index,
                                                          bool delete_empty_groups)
    {
        if (name.empty())
        {
            return ERR("empty string provided when trying to create new pin group for module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        std::unique_ptr<PinGroup<ModulePin>> pin_group_owner(new PinGroup<ModulePin>(id, name, direction, type, ascending, start_index));
        PinGroup<ModulePin>* pin_group = pin_group_owner.get();

        for (ModulePin* pin : pins)
        {
            if (pin == nullptr)
            {
                return ERR("cannot assign 'nullptr' as pin to pin group in module '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
            {
                return ERR("pin '" + pin->get_name() + "' does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
            }

            if (auto res = assign_pin_to_group(pin_group, pin, delete_empty_groups); res.is_error())
            {
                if (auto inner_res = delete_pin_group(pin_group); inner_res.is_error())
                {
                    return ERR(inner_res.get_error());
                }
                return ERR(res.get_error());
            }
        }

        m_pin_groups.push_back(std::move(pin_group_owner));
        m_pin_groups_ordered.push_back(pin_group);
        m_pin_groups_map[id] = pin_group;

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        return OK(pin_group);
    }

    Result<PinGroup<ModulePin>*>
        Module::create_pin_group(const std::string& name, const std::vector<ModulePin*> pins, PinDirection direction, PinType type, bool ascending, u32 start_index, bool delete_empty_groups)
    {
        return create_pin_group(get_unique_pin_group_id(), name, pins, direction, type, ascending, start_index, delete_empty_groups);
    }

    Result<std::monostate> Module::delete_pin_group(PinGroup<ModulePin>* pin_group)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to delete a pin group of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        bool removed_pins = false;

        std::vector<ModulePin*> pins_copy = pin_group->get_pins();
        for (ModulePin* pin : pins_copy)
        {
            removed_pins = true;
            if (auto res = create_pin_group(pin->get_name(), {pin}); res.is_error())
            {
                return ERR(res.get_error());
            }
        }

        if (removed_pins)
        {
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }
        return OK({});
    }

    Result<std::monostate> Module::assign_pin_to_group(PinGroup<ModulePin>* pin_group, ModulePin* pin, bool delete_empty_groups)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to assign a pin to a pin group of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin == nullptr)
        {
            return ERR("'nullptr' given instead of a pin when trying to assign a pin to pin group '" + pin_group->get_name() + "' of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (PinGroup<ModulePin>* pg = pin->get_group().first; pg != nullptr)
        {
            // remove from old group and potentially delete old group if empty
            if (auto res = pg->remove_pin(pin); res.is_error())
            {
                return res;
            }

            if (delete_empty_groups && pg->empty())
            {
                m_pin_groups_map.erase(pg->get_id());
                m_pin_groups_ordered.erase(std::find(m_pin_groups_ordered.begin(), m_pin_groups_ordered.end(), pg));
                m_pin_groups.erase(std::find_if(m_pin_groups.begin(), m_pin_groups.end(), [pg](const std::unique_ptr<PinGroup<ModulePin>>& pg2) { return pg2.get() == pg; }));
            }
        }

        if (auto res = pin_group->assign_pin(pin); res.is_error())
        {
            return res;
        }

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        return OK({});
    }

    Result<std::monostate> Module::move_pin_within_group(PinGroup<ModulePin>* pin_group, ModulePin* pin, u32 new_index)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to move a pin within a pin group of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin == nullptr)
        {
            return ERR("'nullptr' given instead of a pin when trying to move a pin within pin group '" + pin_group->get_name() + "' of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (auto res = pin_group->move_pin(pin, new_index); res.is_error())
        {
            return res;
        }

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        return OK({});
    }

    Result<std::monostate> Module::remove_pin_from_group(PinGroup<ModulePin>* pin_group, ModulePin* pin, bool delete_empty_groups)
    {
        if (pin_group == nullptr)
        {
            return ERR("'nullptr' given instead of a pin group when trying to remove a pin from a pin group of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (pin == nullptr)
        {
            return ERR("'nullptr' given instead of a pin when trying to remove a pin from pin group '" + pin_group->get_name() + "' of module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pin_groups_map.find(pin_group->get_id()); it == m_pin_groups_map.end() || it->second != pin_group)
        {
            return ERR("pin '" + pin->get_name() + "' with ID " + std::to_string(pin->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (const auto it = m_pins_map.find(pin->get_id()); it == m_pins_map.end() || it->second != pin)
        {
            return ERR("pin group '" + pin_group->get_name() + "' with ID " + std::to_string(pin_group->get_id()) + " does not belong to module '" + m_name + "' with ID " + std::to_string(m_id));
        }

        if (auto res = create_pin_group(pin->get_name(), {pin}, pin->get_direction(), pin->get_type(), false, 0, delete_empty_groups); res.is_error())
        {
            return ERR(res.get_error());
        }

        return OK({});
    }

    Result<ModulePin*> Module::assign_pin_net(const u32 pin_id, Net* net, PinDirection direction, const std::string& name, PinType type, bool create_group)
    {
        std::string name_internal;

        if (direction == PinDirection::internal || direction == PinDirection::none)
        {
            return ERR("pin direction '" + enum_to_string(direction) + "' is invalid, cannot assign pin '" + name + "' to net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()));
        }

        if (!name.empty())
        {
            name_internal = name;
        }
        else
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
                    break;
            }
            name_internal = port_prefix + "(" + std::to_string((*index_counter)++) + ")";
        }

        // create pin
        std::unique_ptr<ModulePin> pin_owner(new ModulePin(pin_id, name_internal, net, direction, type));
        ModulePin* pin = pin_owner.get();
        m_pins.push_back(std::move(pin_owner));
        m_pins_map[pin_id] = pin;

        if (create_group)
        {
            // create pin group (every pin must be within exactly one pin group)
            u32 pin_group_id = get_unique_pin_group_id();
            std::unique_ptr<PinGroup<ModulePin>> pin_group_owner(new PinGroup<ModulePin>(pin_group_id, name_internal, pin->get_direction(), pin->get_type()));
            PinGroup<ModulePin>* pin_group = pin_group_owner.get();
            m_pin_groups.push_back(std::move(pin_group_owner));
            m_pin_groups_ordered.push_back(pin_group);
            m_pin_groups_map[pin_group_id] = pin_group;

            if (auto res = assign_pin_to_group(pin_group, pin); res.is_error())    // triggers 'pin_changed' event
            {
                return ERR(res.get_error());
            }
        }
        else
        {
            m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        }

        return OK(pin);
    }

    Result<std::monostate> Module::remove_pin_net(Net* net)
    {
        ModulePin* pin;
        if (auto res = get_pin_by_net(net); res.is_error())
        {
            return ERR(res.get_error());
        }
        else
        {
            pin = res.get();
        }

        PinGroup<ModulePin>* pin_group = pin->get_group().first;
        assert(pin_group != nullptr);

        if (auto res = pin_group->remove_pin(pin); res.is_error())
        {
            return res;
        }

        if (pin_group->empty())
        {
            // remove pin group
            m_pin_groups_map.erase(pin_group->get_id());
            m_pin_groups_ordered.erase(std::find(m_pin_groups_ordered.begin(), m_pin_groups_ordered.end(), pin_group));
            m_pin_groups.erase(std::find_if(m_pin_groups.begin(), m_pin_groups.end(), [pin_group](const std::unique_ptr<PinGroup<ModulePin>>& pg) { return pg.get() == pin_group; }));
        }

        m_pins_map.erase(pin->get_id());
        m_pins.erase(std::find_if(m_pins.begin(), m_pins.end(), [pin](const std::unique_ptr<ModulePin>& p) { return p.get() == pin; }));

        m_event_handler->notify(ModuleEvent::event::pin_changed, this);
        return OK({});
    }
}    // namespace hal
