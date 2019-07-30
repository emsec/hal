#include "netlist_watcher/netlist_watcher.h"

#include "gui_globals.h"

netlist_watcher::netlist_watcher(QObject* parent) : QObject(parent)
{
    netlist_event_handler::register_callback(
                "netlist_watcher",
                std::function<void(netlist_event_handler::event, std::shared_ptr<netlist>, u32)>
                (std::bind(&netlist_watcher::handle_netlist_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    net_event_handler::register_callback(
                "netlist_watcher",
                std::function<void(net_event_handler::event, std::shared_ptr<net>, u32)>
                (std::bind(&netlist_watcher::handle_net_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    gate_event_handler::register_callback(
                "netlist_watcher",
                std::function<void(gate_event_handler::event, std::shared_ptr<gate>, u32)>
                (std::bind(&netlist_watcher::handle_gate_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    module_event_handler::register_callback(
                "netlist_watcher",
                std::function<void(module_event_handler::event, std::shared_ptr<module>, u32)>
                (std::bind(&netlist_watcher::handle_module_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    reset();
}

netlist_watcher::~netlist_watcher()
{
    netlist_event_handler::unregister_callback("netlist_watcher");
    net_event_handler::unregister_callback("netlist_watcher");
    gate_event_handler::unregister_callback("netlist_watcher");
    module_event_handler::unregister_callback("netlist_watcher");
}

void netlist_watcher::handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data)
{
    handle_netlist_modified();
}

void netlist_watcher::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data)
{
    handle_netlist_modified();
}

void netlist_watcher::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data)
{
    handle_netlist_modified();
}

void netlist_watcher::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data)
{
    handle_netlist_modified();
}

void netlist_watcher::reset()
{
    m_notified = false;
}

void netlist_watcher::handle_netlist_modified()
{
    if (!m_notified)
    {
        m_notified = true;
        g_file_status_manager.netlist_changed();
    }
}
