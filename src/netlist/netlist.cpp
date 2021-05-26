#include "hal_core/netlist/netlist.h"

#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    Netlist::Netlist(const GateLibrary* library) : m_gate_library(library)
    {
        m_event_handler    = std::make_unique<EventHandler>();
        m_manager          = new NetlistInternalManager(this, m_event_handler.get());
        m_netlist_id       = 1;
        m_next_gate_id     = 1;
        m_next_net_id      = 1;
        m_next_module_id   = 1;
        m_next_grouping_id = 1;
        m_top_module       = nullptr;    // this triggers the internal manager to allow creation of a module without parent
        m_top_module       = create_module("top_module", nullptr);
    }

    Netlist::~Netlist()
    {
        delete m_manager;
    }

    bool Netlist::operator==(const Netlist& other) const
    {
        if (m_file_name != other.get_input_filename() || m_design_name != other.get_design_name() || m_device_name != other.get_device_name())
        {
            log_info("netlist", "the netlists with IDs {} and {} are not equal due to an unequal file name, design name, or device name.", m_netlist_id, other.get_id());
            return false;
        }

        if (m_gate_library != other.get_gate_library())
        {
            log_info("netlist", "the netlists with IDs {} and {} are not equal due to an unequal gate library.", m_netlist_id, other.get_id());
            return false;
        }

        if (m_gates.size() != other.get_gates().size() || m_nets.size() != other.get_nets().size() || m_modules.size() != other.get_modules().size())
        {
            log_info("netlist", "the netlists with IDs {} and {} are not equal due to an unequal number of gates, nets, or modules.", m_netlist_id, other.get_id());
            return false;
        }

        for (const Net* net : other.get_nets())
        {
            if (const auto it = m_nets_map.find(net->get_id()); it == m_nets_map.end() || *it->second != *net)
            {
                log_info("netlist", "the netlists with IDs {} and {} are not equal due to unequal nets.", m_netlist_id, other.get_id());
                return false;
            }
        }

        // gates check included within modules

        // modules are checked recursively
        if (*m_top_module != *other.get_top_module())
        {
            log_info("netlist", "the netlists with IDs {} and {} are not equal due to unequal modules.", m_netlist_id, other.get_id());
            return false;
        }

        return true;
    }

    bool Netlist::operator!=(const Netlist& other) const
    {
        return !operator==(other);
    }

    u32 Netlist::get_id() const
    {
        return m_netlist_id;
    }

    void Netlist::set_id(const u32 id)
    {
        if (id != m_netlist_id)
        {
            auto old_id  = m_netlist_id;
            m_netlist_id = id;
            m_event_handler->notify(NetlistEvent::event::id_changed, this, old_id);
        }
    }

    std::filesystem::path Netlist::get_input_filename() const
    {
        return m_file_name;
    }

    void Netlist::set_input_filename(const std::filesystem::path& input_filename)
    {
        if (input_filename != m_file_name)
        {
            m_file_name = input_filename;
            m_event_handler->notify(NetlistEvent::event::input_filename_changed, this);
        }
    }

    std::string Netlist::get_design_name() const
    {
        return m_design_name;
    }

    void Netlist::set_design_name(const std::string& design_name)
    {
        if (design_name != m_design_name)
        {
            m_design_name = design_name;
            m_event_handler->notify(NetlistEvent::event::design_name_changed, this);
        }
    }

    std::string Netlist::get_device_name() const
    {
        return m_device_name;
    }

    void Netlist::set_device_name(const std::string& device_name)
    {
        if (device_name != m_device_name)
        {
            m_device_name = device_name;
            m_event_handler->notify(NetlistEvent::event::device_name_changed, this);
        }
    }

    const GateLibrary* Netlist::get_gate_library() const
    {
        return m_gate_library;
    }

    EventHandler* Netlist::get_event_handler() const
    {
        return m_event_handler.get();
    }

    void Netlist::clear_caches()
    {
        m_manager->clear_caches();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    u32 Netlist::get_unique_module_id()
    {
        if (!m_free_module_ids.empty())
        {
            return *(m_free_module_ids.begin());
        }
        while (m_used_module_ids.find(m_next_module_id) != m_used_module_ids.end())
        {
            m_next_module_id++;
        }
        return m_next_module_id;
    }

    Module* Netlist::create_module(const u32 id, const std::string& name, Module* parent, const std::vector<Gate*>& gates)
    {
        auto m = m_manager->create_module(id, parent, name);
        if (m == nullptr)
        {
            return nullptr;
        }
        for (auto g : gates)
        {
            m->assign_gate(g);
        }
        return m;
    }

    Module* Netlist::create_module(const std::string& name, Module* parent, const std::vector<Gate*>& gates)
    {
        return create_module(get_unique_module_id(), name, parent, gates);
    }

    bool Netlist::delete_module(Module* module)
    {
        return m_manager->delete_module(module);
    }

    Module* Netlist::get_top_module() const
    {
        return m_top_module;
    }

    Module* Netlist::get_module_by_id(u32 id) const
    {
        auto it = m_modules_map.find(id);
        if (it == m_modules_map.end())
        {
            log_error("netlist", "there is no module with ID {} in the netlist with ID {}.", id, m_netlist_id);
            return nullptr;
        }
        return it->second.get();
    }

    const std::vector<Module*>& Netlist::get_modules() const
    {
        return m_modules;
    }

    std::vector<Module*> Netlist::get_modules(const std::function<bool(Module*)>& filter) const
    {
        if (!filter)
        {
            return m_modules;
        }
        std::vector<Module*> res;
        for (auto module : m_modules)
        {
            if (!filter(module))
            {
                continue;
            }
            res.push_back(module);
        }
        return res;
    }

    bool Netlist::is_module_in_netlist(Module* module) const
    {
        return (module != nullptr) && (m_modules_set.find(module) != m_modules_set.end());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    u32 Netlist::get_unique_gate_id()
    {
        if (!m_free_gate_ids.empty())
        {
            return *(m_free_gate_ids.begin());
        }
        while (m_used_gate_ids.find(m_next_gate_id) != m_used_gate_ids.end())
        {
            m_next_gate_id++;
        }
        return m_next_gate_id;
    }

    Gate* Netlist::create_gate(const u32 id, GateType* gt, const std::string& name, i32 x, i32 y)
    {
        return m_manager->create_gate(id, gt, name, x, y);
    }

    Gate* Netlist::create_gate(GateType* gt, const std::string& name, i32 x, i32 y)
    {
        return create_gate(get_unique_gate_id(), gt, name, x, y);
    }

    bool Netlist::delete_gate(Gate* gate)
    {
        return m_manager->delete_gate(gate);
    }

    bool Netlist::is_gate_in_netlist(Gate* gate) const
    {
        return gate != nullptr && m_gates_set.find(gate) != m_gates_set.end();
    }

    Gate* Netlist::get_gate_by_id(const u32 gate_id) const
    {
        if (auto it = m_gates_map.find(gate_id); it != m_gates_map.end())
        {
            return it->second.get();
        }

        log_error("netlist", "there is no gate with ID {} in the netlist with ID {}.", gate_id, m_netlist_id);
        return nullptr;
    }

    const std::vector<Gate*>& Netlist::get_gates() const
    {
        return m_gates;
    }

    std::vector<Gate*> Netlist::get_gates(const std::function<bool(Gate*)>& filter) const
    {
        if (!filter)
        {
            return m_gates;
        }
        std::vector<Gate*> res;
        for (Gate* g : m_gates)
        {
            if (!filter(g))
            {
                continue;
            }
            res.push_back(g);
        }

        return res;
    }

    bool Netlist::mark_vcc_gate(Gate* gate)
    {
        if (!is_gate_in_netlist(gate))
        {
            return false;
        }
        if (is_vcc_gate(gate))
        {
            log_debug("netlist", "gate '{}' with ID {} is already registered as global VCC gate in the netlist with ID {}.", gate->get_name(), gate->get_id(), m_netlist_id);
            return true;
        }
        m_vcc_gates.push_back(gate);
        m_event_handler->notify(NetlistEvent::event::marked_global_vcc, this, gate->get_id());
        return true;
    }

    bool Netlist::mark_gnd_gate(Gate* gate)
    {
        if (!is_gate_in_netlist(gate))
        {
            return false;
        }
        if (is_gnd_gate(gate))
        {
            log_debug("netlist", "gate '{}' with ID {} is already registered as global GND gate in the netlist with ID {}.", gate->get_name(), gate->get_id(), m_netlist_id);
            return true;
        }
        m_gnd_gates.push_back(gate);
        m_event_handler->notify(NetlistEvent::event::marked_global_gnd, this, gate->get_id());
        return true;
    }

    bool Netlist::unmark_vcc_gate(Gate* gate)
    {
        if (!is_gate_in_netlist(gate))
        {
            return false;
        }
        auto it = std::find(m_vcc_gates.begin(), m_vcc_gates.end(), gate);
        if (it == m_vcc_gates.end())
        {
            log_debug("netlist", "gate '{}' with ID {} is not registered as global VCC gate in the netlist with ID {}.", gate->get_name(), gate->get_id(), m_netlist_id);
            return false;
        }
        m_vcc_gates.erase(it);
        m_event_handler->notify(NetlistEvent::event::unmarked_global_vcc, this, gate->get_id());
        return true;
    }

    bool Netlist::unmark_gnd_gate(Gate* gate)
    {
        if (!is_gate_in_netlist(gate))
        {
            return false;
        }
        auto it = std::find(m_gnd_gates.begin(), m_gnd_gates.end(), gate);
        if (it == m_gnd_gates.end())
        {
            log_debug("netlist", "gate '{}' with ID {} is not registered as global GND gate in the netlist with ID {}.", gate->get_name(), gate->get_id(), m_netlist_id);
            return false;
        }
        m_gnd_gates.erase(it);
        m_event_handler->notify(NetlistEvent::event::unmarked_global_gnd, this, gate->get_id());
        return true;
    }

    bool Netlist::is_vcc_gate(const Gate* gate) const
    {
        return (std::find(m_vcc_gates.begin(), m_vcc_gates.end(), gate) != m_vcc_gates.end());
    }

    bool Netlist::is_gnd_gate(const Gate* gate) const
    {
        return (std::find(m_gnd_gates.begin(), m_gnd_gates.end(), gate) != m_gnd_gates.end());
    }

    const std::vector<Gate*>& Netlist::get_vcc_gates() const
    {
        return m_vcc_gates;
    }

    const std::vector<Gate*>& Netlist::get_gnd_gates() const
    {
        return m_gnd_gates;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    u32 Netlist::get_unique_net_id()
    {
        if (!m_free_net_ids.empty())
        {
            return *(m_free_net_ids.begin());
        }
        while (m_used_net_ids.find(m_next_net_id) != m_used_net_ids.end())
        {
            m_next_net_id++;
        }
        return m_next_net_id;
    }

    Net* Netlist::create_net(const u32 id, const std::string& name)
    {
        return m_manager->create_net(id, name);
    }

    Net* Netlist::create_net(const std::string& name)
    {
        return m_manager->create_net(get_unique_net_id(), name);
    }

    bool Netlist::delete_net(Net* n)
    {
        return m_manager->delete_net(n);
    }

    bool Netlist::is_net_in_netlist(Net* n) const
    {
        return n != nullptr && m_nets_set.find(n) != m_nets_set.end();
    }

    Net* Netlist::get_net_by_id(u32 net_id) const
    {
        auto it = m_nets_map.find(net_id);
        if (it == m_nets_map.end())
        {
            log_error("netlist", "there is no net with ID {} in the netlist with ID {}.", net_id, m_netlist_id);
            return nullptr;
        }
        return it->second.get();
    }

    const std::vector<Net*>& Netlist::get_nets() const
    {
        return m_nets;
    }

    std::vector<Net*> Netlist::get_nets(const std::function<bool(Net*)>& filter) const
    {
        if (!filter)
        {
            return m_nets;
        }
        std::vector<Net*> res;
        for (auto net : m_nets)
        {
            if (!filter(net))
            {
                continue;
            }
            res.push_back(net);
        }
        return res;
    }

    bool Netlist::mark_global_input_net(Net* n)
    {
        if (!is_net_in_netlist(n))
        {
            return false;
        }
        if (is_global_input_net(n))
        {
            log_debug("netlist", "net '{}' with ID {} is already registered as global input net in the netlist with ID {}.", n->get_name(), n->get_id(), m_netlist_id);
            return true;
        }
        m_global_input_nets.push_back(n);

        // mark module caches as dirty
        std::unordered_set<Module*> affected_modules;
        for (Endpoint* ep : n->get_sources())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }
        for (Endpoint* ep : n->get_destinations())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }

        for (Module* m : affected_modules)
        {
            m->set_cache_dirty();
        }

        m_event_handler->notify(NetlistEvent::event::marked_global_input, this, n->get_id());
        return true;
    }

    bool Netlist::mark_global_output_net(Net* n)
    {
        if (!is_net_in_netlist(n))
        {
            return false;
        }
        if (is_global_output_net(n))
        {
            log_debug("netlist", "net '{}' with ID {} is already registered as global output net in the netlist with ID {}.", n->get_name(), n->get_id(), m_netlist_id);
            return true;
        }
        m_global_output_nets.push_back(n);

        // mark module caches as dirty
        std::unordered_set<Module*> affected_modules;
        for (Endpoint* ep : n->get_sources())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }
        for (Endpoint* ep : n->get_destinations())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }

        for (Module* m : affected_modules)
        {
            m->set_cache_dirty();
        }

        m_event_handler->notify(NetlistEvent::event::marked_global_output, this, n->get_id());
        return true;
    }

    bool Netlist::unmark_global_input_net(Net* n)
    {
        if (!is_net_in_netlist(n))
        {
            return false;
        }
        auto it = std::find(m_global_input_nets.begin(), m_global_input_nets.end(), n);
        if (it == m_global_input_nets.end())
        {
            log_debug("netlist", "net '{}' with ID {} is not registered as global input net in the netlist with ID {}.", n->get_name(), n->get_id(), m_netlist_id);
            return false;
        }
        m_global_input_nets.erase(it);

        // mark module caches as dirty
        std::unordered_set<Module*> affected_modules;
        for (Endpoint* ep : n->get_sources())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }
        for (Endpoint* ep : n->get_destinations())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }

        for (Module* m : affected_modules)
        {
            m->set_cache_dirty();
        }

        m_event_handler->notify(NetlistEvent::event::unmarked_global_input, this, n->get_id());
        return true;
    }

    bool Netlist::unmark_global_output_net(Net* n)
    {
        if (!is_net_in_netlist(n))
        {
            return false;
        }
        auto it = std::find(m_global_output_nets.begin(), m_global_output_nets.end(), n);
        if (it == m_global_output_nets.end())
        {
            log_debug("netlist", "net '{}' with ID {} is not registered as global output net in the netlist with ID {}.", n->get_name(), n->get_id(), m_netlist_id);
            return false;
        }
        m_global_output_nets.erase(it);

        // mark module caches as dirty
        std::unordered_set<Module*> affected_modules;
        for (Endpoint* ep : n->get_sources())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }
        for (Endpoint* ep : n->get_destinations())
        {
            affected_modules.insert(ep->get_gate()->get_module());
        }

        for (Module* m : affected_modules)
        {
            m->set_cache_dirty();
        }

        m_event_handler->notify(NetlistEvent::event::unmarked_global_output, this, n->get_id());
        return true;
    }

    bool Netlist::is_global_input_net(const Net* n) const
    {
        return (std::find(m_global_input_nets.begin(), m_global_input_nets.end(), n) != m_global_input_nets.end());
    }

    bool Netlist::is_global_output_net(const Net* n) const
    {
        return (std::find(m_global_output_nets.begin(), m_global_output_nets.end(), n) != m_global_output_nets.end());
    }

    const std::vector<Net*>& Netlist::get_global_input_nets() const
    {
        return m_global_input_nets;
    }

    const std::vector<Net*>& Netlist::get_global_output_nets() const
    {
        return m_global_output_nets;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    u32 Netlist::get_unique_grouping_id()
    {
        if (!m_free_grouping_ids.empty())
        {
            return *(m_free_grouping_ids.begin());
        }
        while (m_used_grouping_ids.find(m_next_grouping_id) != m_used_grouping_ids.end())
        {
            m_next_grouping_id++;
        }
        return m_next_grouping_id;
    }

    Grouping* Netlist::create_grouping(const u32 id, const std::string& name)
    {
        return m_manager->create_grouping(id, name);
    }

    Grouping* Netlist::create_grouping(const std::string& name)
    {
        return m_manager->create_grouping(get_unique_grouping_id(), name);
    }

    bool Netlist::delete_grouping(Grouping* g)
    {
        return m_manager->delete_grouping(g);
    }

    bool Netlist::is_grouping_in_netlist(Grouping* n) const
    {
        return n != nullptr && m_groupings_set.find(n) != m_groupings_set.end();
    }

    Grouping* Netlist::get_grouping_by_id(u32 grouping_id) const
    {
        auto it = m_groupings_map.find(grouping_id);
        if (it == m_groupings_map.end())
        {
            log_error("netlist", "there is no grouping with ID {} in the netlist with ID {}.", grouping_id, m_netlist_id);
            return nullptr;
        }
        return it->second.get();
    }

    std::vector<Grouping*> Netlist::get_groupings(const std::function<bool(Grouping*)>& filter) const
    {
        if (!filter)
        {
            return m_groupings;
        }
        std::vector<Grouping*> res;
        for (auto grouping : m_groupings)
        {
            if (!filter(grouping))
            {
                continue;
            }
            res.push_back(grouping);
        }
        return res;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    u32 Netlist::get_next_gate_id() const
    {
        return m_next_gate_id;
    }

    void Netlist::set_next_gate_id(const u32 id)
    {
        m_next_gate_id = id;
    }

    std::set<u32> Netlist::get_used_gate_ids() const
    {
        return m_used_gate_ids;
    }

    void Netlist::set_used_gate_ids(const std::set<u32> ids)
    {
        m_used_gate_ids = ids;
    }

    std::set<u32> Netlist::get_free_gate_ids() const
    {
        return m_free_gate_ids;
    }

    void Netlist::set_free_gate_ids(const std::set<u32> ids)
    {
        m_free_gate_ids = ids;
    }

    u32 Netlist::get_next_net_id() const
    {
        return m_next_net_id;
    }

    void Netlist::set_next_net_id(const u32 id)
    {
        m_next_net_id = id;
    }

    std::set<u32> Netlist::get_used_net_ids() const
    {
        return m_used_net_ids;
    }

    void Netlist::set_used_net_ids(const std::set<u32> ids)
    {
        m_used_net_ids = ids;
    }

    std::set<u32> Netlist::get_free_net_ids() const
    {
        return m_free_net_ids;
    }

    void Netlist::set_free_net_ids(const std::set<u32> ids)
    {
        m_free_net_ids = ids;
    }

    u32 Netlist::get_next_module_id() const
    {
        return m_next_module_id;
    }

    void Netlist::set_next_module_id(const u32 id)
    {
        m_next_module_id = id;
    }

    std::set<u32> Netlist::get_used_module_ids() const
    {
        return m_used_module_ids;
    }

    void Netlist::set_used_module_ids(const std::set<u32> ids)
    {
        m_used_module_ids = ids;
    }

    std::set<u32> Netlist::get_free_module_ids() const
    {
        return m_free_module_ids;
    }

    void Netlist::set_free_module_ids(const std::set<u32> ids)
    {
        m_free_module_ids = ids;
    }

    u32 Netlist::get_next_grouping_id() const
    {
        return m_next_grouping_id;
    }

    void Netlist::set_next_grouping_id(const u32 id)
    {
        m_next_grouping_id = id;
    }

    std::set<u32> Netlist::get_used_grouping_ids() const
    {
        return m_used_grouping_ids;
    }

    void Netlist::set_used_grouping_ids(const std::set<u32> ids)
    {
        m_used_grouping_ids = ids;
    }

    std::set<u32> Netlist::get_free_grouping_ids() const
    {
        return m_free_grouping_ids;
    }

    void Netlist::set_free_grouping_ids(const std::set<u32> ids)
    {
        m_free_grouping_ids = ids;
    }

}    // namespace hal
