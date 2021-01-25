#include "hal_core/netlist/event_handler.h"

#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    EventHandler::EventHandler() {
        netlist_event_enabled = true;
        gate_event_enabled = true;
        net_event_enabled = true;
        module_event_enabled = true;
    }

    void EventHandler::netlist_event_enable(bool flag)
    {
        netlist_event_enabled = flag;
    }

    void EventHandler::notify(NetlistEvent::event c, Netlist* netlist, u32 associated_data)
    {
        if (netlist_event_enabled)
        {
            m_netlist_callback(c, netlist, associated_data);
        }
    }

    void EventHandler::notify(GateEvent::event c, Gate* gate, u32 associated_data)
    {
        if (gate_event_enabled)
        {
            m_gate_callback(c, gate, associated_data);
        }
    }

    void EventHandler::notify(NetEvent::event c, Net* net, u32 associated_data)
    {
        if (net_event_enabled)
        {
            m_net_callback(c, net, associated_data);
        }
    }

    void EventHandler::notify(ModuleEvent::event c, Module* module, u32 associated_data)
    {
        if (module_event_enabled)
        {
            m_module_callback(c, module, associated_data);
        }
    }

    void EventHandler::register_callback(const std::string& name, std::function<void(NetlistEvent::event, Netlist*, u32)> function)
    {
        m_netlist_callback.add_callback(name, function);
    }

    void EventHandler::unregister_callback(const std::string& name)
    {
        m_netlist_callback.remove_callback(name);
    }
}    // namespace hal
