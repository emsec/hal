#include "hal_core/netlist/module.h"

#include "hal_core/netlist/event_system/module_event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    Module::Module(u32 id, Module* parent, const std::string& name, NetlistInternalManager* internal_manager)
    {
        m_internal_manager = internal_manager;
        m_id               = id;
        m_parent           = parent;
        m_name             = name;
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
            log_error("module", "empty name is not allowed.");
            return;
        }
        if (name != m_name)
        {
            m_name = name;

            module_event_handler::notify(module_event_handler::event::name_changed, this);
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

            module_event_handler::notify(module_event_handler::event::type_changed, this);
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

    bool Module::set_parent_module(Module* new_parent)
    {
        if (new_parent == this)
        {
            log_error("module", "can not set module as its own parent");
            return false;
        }

        if (m_parent == nullptr)
        {
            log_error("module", "no parent can be assigned to the top module");
            return false;
        }

        if (new_parent == nullptr)
        {
            log_error("module", "cannot reassign top module");
            return false;
        }

        if (!get_netlist()->is_module_in_netlist(new_parent))
        {
            log_error("module", "module must be in the current netlist");
            return false;
        }

        auto children = get_submodules(nullptr, true);
        if (std::find(children.begin(), children.end(), new_parent) != children.end())
        {
            new_parent->set_parent_module(m_parent);
        }

        m_parent->m_submodules_map.erase(m_id);
        m_parent->m_submodules.erase(std::find(m_parent->m_submodules.begin(), m_parent->m_submodules.end(), this));

        m_parent->m_input_nets_dirty    = true;
        m_parent->m_output_nets_dirty   = true;
        m_parent->m_internal_nets_dirty = true;

        module_event_handler::notify(module_event_handler::event::submodule_removed, m_parent, m_id);

        m_parent = new_parent;

        m_parent->m_submodules_map[m_id] = this;
        m_parent->m_submodules.push_back(this);

        m_parent->m_input_nets_dirty    = true;
        m_parent->m_output_nets_dirty   = true;
        m_parent->m_internal_nets_dirty = true;

        module_event_handler::notify(module_event_handler::event::parent_changed, this);
        module_event_handler::notify(module_event_handler::event::submodule_added, m_parent, m_id);

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

    Netlist* Module::get_netlist() const
    {
        return m_internal_manager->m_netlist;
    }

    bool Module::assign_gate(Gate* gate)
    {
        m_input_nets_dirty    = true;
        m_output_nets_dirty   = true;
        m_internal_nets_dirty = true;
        return m_internal_manager->module_assign_gate(this, gate);
    }

    bool Module::remove_gate(Gate* gate)
    {
        m_input_nets_dirty    = true;
        m_output_nets_dirty   = true;
        m_internal_nets_dirty = true;
        return m_internal_manager->module_remove_gate(this, gate);
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

    std::vector<Net*> Module::get_input_nets() const
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

    std::vector<Net*> Module::get_output_nets() const
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

    std::vector<Net*> Module::get_internal_nets() const
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

    void Module::set_input_port_name(Net* input_net, const std::string& port_name)
    {
        if (input_net == nullptr)
        {
            log_warning("module", "nullptr given as input net of module {} with id {}.", this->get_name(), this->get_id());
            return;
        }

        auto input_nets = get_input_nets();

        if (auto it = std::find(input_nets.begin(), input_nets.end(), input_net); it == input_nets.end())
        {
            log_warning(
                "module", "net '{}' with id {} is not an input net of module '{}' with id {}, ignoring port assignment", input_net->get_name(), input_net->get_id(), this->get_name(), this->get_id());
            return;
        }

        m_named_input_nets.insert(input_net);
        m_input_net_to_port_name[input_net] = port_name;

        module_event_handler::notify(module_event_handler::event::input_port_name_changed, this, input_net->get_id());
    }

    void Module::set_output_port_name(Net* output_net, const std::string& port_name)
    {
        if (output_net == nullptr)
        {
            log_warning("module", "nullptr given as output net of module {} with id {}.", this->get_name(), this->get_id());
            return;
        }

        auto output_nets = get_output_nets();

        if (auto it = std::find(output_nets.begin(), output_nets.end(), output_net); it == output_nets.end())
        {
            log_warning("module",
                        "net '{}' with id {} is not an output net of module '{}' with id {}, ignoring port assignment",
                        output_net->get_name(),
                        output_net->get_id(),
                        this->get_name(),
                        this->get_id());
            return;
        }

        m_named_output_nets.insert(output_net);
        m_output_net_to_port_name[output_net] = port_name;

        module_event_handler::notify(module_event_handler::event::output_port_name_changed, this, output_net->get_id());
    }

    std::string Module::get_input_port_name(Net* net)
    {
        if (net == nullptr)
        {
            log_warning("module", "nullptr given as input net of module {} with id {}.", this->get_name(), this->get_id());
            return "";
        }

        auto input_nets = get_input_nets();

        if (auto it = std::find(input_nets.begin(), input_nets.end(), net); it == input_nets.end())
        {
            log_warning("module", "net '{}' with id {} is not an input net of module '{}' with id {}.", net->get_name(), net->get_id(), this->get_name(), this->get_id());
            return "";
        }

        std::string port_name;
        if (auto it = m_input_net_to_port_name.find(net); it != m_input_net_to_port_name.end())
        {
            port_name = it->second;
        }
        else
        {
            port_name = "I(" + std::to_string(m_next_input_port_id++) + ")";
            m_named_input_nets.insert(net);
            m_input_net_to_port_name.emplace(net, port_name);
        }

        return port_name;
    }

    std::string Module::get_output_port_name(Net* net)
    {
        if (net == nullptr)
        {
            log_warning("module", "nullptr given as output net of module {} with id {}.", this->get_name(), this->get_id());
            return "";
        }
        auto output_nets = get_output_nets();

        if (auto it = std::find(output_nets.begin(), output_nets.end(), net); it == output_nets.end())
        {
            log_warning("module", "net '{}' with id {} is not an output net of module '{}' with id {}.", net->get_name(), net->get_id(), this->get_name(), this->get_id());
            return "";
        }

        std::string port_name;
        if (auto it = m_output_net_to_port_name.find(net); it != m_output_net_to_port_name.end())
        {
            port_name = it->second;
        }
        else
        {
            port_name = "O(" + std::to_string(m_next_output_port_id++) + ")";
            m_named_output_nets.insert(net);
            m_output_net_to_port_name.emplace(net, port_name);
        }

        return port_name;
    }

    Net* Module::get_input_port_net(const std::string& port_name)
    {
        for (const auto& [net, name] : m_input_net_to_port_name)
        {
            if (name == port_name)
            {
                return net;
            }
        }

        log_warning("module", "port '{}' is not an input port of module '{}' with id {}.", port_name, this->get_name(), this->get_id());
        return nullptr;
    }

    Net* Module::get_output_port_net(const std::string& port_name)
    {
        for (const auto& [net, name] : m_output_net_to_port_name)
        {
            if (name == port_name)
            {
                return net;
            }
        }

        log_warning("module", "port '{}' is not an output port of module '{}' with id {}.", port_name, this->get_name(), this->get_id());
        return nullptr;
    }

    const std::map<Net*, std::string>& Module::get_input_port_names() const
    {
        auto input_nets = get_input_nets();

        std::vector<Net*> diff;

        // find nets that are still in the port map but no longer an input net
        std::set_difference(m_named_input_nets.begin(), m_named_input_nets.end(), input_nets.begin(), input_nets.end(), std::back_inserter(diff));
        for (auto net : diff)
        {
            m_named_input_nets.erase(net);
            m_input_net_to_port_name.erase(net);
        }

        diff.clear();

        // find nets that are input nets but have not yet been assigned a port name
        std::set_difference(input_nets.begin(), input_nets.end(), m_named_input_nets.begin(), m_named_input_nets.end(), std::back_inserter(diff));
        for (auto net : diff)
        {
            auto port_name = "I(" + std::to_string(m_next_input_port_id++) + ")";
            m_named_input_nets.insert(net);
            m_input_net_to_port_name.emplace(net, port_name);
        }

        return m_input_net_to_port_name;
    }

    const std::map<Net*, std::string>& Module::get_output_port_names() const
    {
        auto output_nets = get_output_nets();
        std::set<Net*> diff;

        // find nets that are still in the port map but no longer an output net
        std::set_difference(m_named_output_nets.begin(), m_named_output_nets.end(), output_nets.begin(), output_nets.end(), std::inserter(diff, diff.begin()));
        for (auto net : diff)
        {
            m_named_output_nets.erase(net);
            m_output_net_to_port_name.erase(net);
        }

        diff.clear();

        // find nets that are output nets but have not yet been assigned a port name
        std::set_difference(output_nets.begin(), output_nets.end(), m_named_output_nets.begin(), m_named_output_nets.end(), std::inserter(diff, diff.begin()));
        for (auto net : diff)
        {
            auto port_name = "O(" + std::to_string(m_next_output_port_id++) + ")";
            m_named_output_nets.insert(net);
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
}    // namespace hal
