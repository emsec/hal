#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/event_system/event_log.h"

#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    EventHandler::EventHandler()
        : netlist_event_enabled(true),
          module_event_enabled(true),
          gate_event_enabled(true),
          net_event_enabled(true),
          grouping_event_enabled(true)
    {;}

    void EventHandler::event_enable_all(bool flag)
    {
        netlist_event_enabled  = flag;
        module_event_enabled   = flag;
        gate_event_enabled     = flag;
        net_event_enabled      = flag;
        grouping_event_enabled = flag;
    }

    void EventHandler::notify(NetlistEvent::event c, Netlist* netlist, u32 associated_data)
    {
        if (netlist_event_enabled)
        {
            m_netlist_callback(c, netlist, associated_data);
            event_log::handle_netlist_event(c, netlist, associated_data);
        }
    }

    void EventHandler::notify(GateEvent::event c, Gate* gate, u32 associated_data)
    {
        if (gate_event_enabled)
        {
            m_gate_callback(c, gate, associated_data);
            event_log::handle_gate_event(c, gate, associated_data);
        }
    }

    void EventHandler::notify(NetEvent::event c, Net* net, u32 associated_data)
    {
        if (net_event_enabled)
        {
            m_net_callback(c, net, associated_data);
            event_log::handle_net_event(c, net, associated_data);
        }
    }

    void EventHandler::notify(ModuleEvent::event c, Module* module, u32 associated_data)
    {
//        ModuleEvent::dump(c, true);
        if (module_event_enabled)
        {
            m_module_callback(c, module, associated_data);
            event_log::handle_submodule_event(c, module, associated_data);
        }
    }

    void EventHandler::notify(GroupingEvent::event c, Grouping* grouping, u32 associated_data)
    {
        if (grouping_event_enabled)
        {
            m_grouping_callback(c, grouping, associated_data);
        }
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(GateEvent::event, Gate*, u32)> function)
    {
        m_gate_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(GroupingEvent::event, Grouping*, u32)> function)
    {
        m_grouping_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(ModuleEvent::event, Module*, u32)> function)
    {
        m_module_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(NetEvent::event, Net*, u32)> function)
    {
        m_net_callback.add_callback(name, function);
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(NetlistEvent::event, Netlist*, u32)> function)
    {
        m_netlist_callback.add_callback(name, function);
    }

    void EventHandler::unregister_callback(const std::string& name)
    {
        m_netlist_callback.remove_callback(name);
        m_module_callback.remove_callback(name);
        m_gate_callback.remove_callback(name);
        m_net_callback.remove_callback(name);
        m_grouping_callback.remove_callback(name);
    }
}    // namespace hal
