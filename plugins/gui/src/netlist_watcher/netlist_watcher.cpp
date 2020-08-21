#include "netlist_watcher/netlist_watcher.h"

#include "gui_globals.h"

namespace hal
{
    NetlistWatcher::NetlistWatcher(QObject* parent) : QObject(parent)
    {
        netlist_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(netlist_event_handler::event, Netlist*, u32)>
                    (std::bind(&NetlistWatcher::handle_netlist_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        net_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(net_event_handler::event, Net*, u32)>
                    (std::bind(&NetlistWatcher::handle_net_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        gate_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(gate_event_handler::event, Gate*, u32)>
                    (std::bind(&NetlistWatcher::handle_gate_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        module_event_handler::register_callback(
                    "NetlistWatcher",
                    std::function<void(module_event_handler::event, Module*, u32)>
                    (std::bind(&NetlistWatcher::handle_module_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

        reset();
    }

    NetlistWatcher::~NetlistWatcher()
    {
        netlist_event_handler::unregister_callback("NetlistWatcher");
        net_event_handler::unregister_callback("NetlistWatcher");
        gate_event_handler::unregister_callback("NetlistWatcher");
        module_event_handler::unregister_callback("NetlistWatcher");
    }

    void NetlistWatcher::handle_netlist_event(netlist_event_handler::event ev, Netlist* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handle_netlist_modified();
    }

    void NetlistWatcher::handle_module_event(module_event_handler::event ev, Module* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handle_netlist_modified();
    }

    void NetlistWatcher::handle_gate_event(gate_event_handler::event ev, Gate* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handle_netlist_modified();
    }

    void NetlistWatcher::handle_net_event(net_event_handler::event ev, Net* object, u32 associated_data)
    {
        Q_UNUSED(ev);
        Q_UNUSED(object);
        Q_UNUSED(associated_data);
        handle_netlist_modified();
    }

    void NetlistWatcher::reset()
    {
        m_notified = false;
    }

    void NetlistWatcher::handle_netlist_modified()
    {
        if (!m_notified)
        {
            m_notified = true;
            g_file_status_manager.netlist_changed();
        }
    }
}
