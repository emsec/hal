#include "netlist_watcher/netlist_watcher.h"

#include "gui_globals.h"

netlist_watcher::netlist_watcher(QObject* parent) : QObject(parent)
{
    connect(&g_graph_relay, &graph_relay::gate_event, this, &netlist_watcher::handle_gate_event);
    connect(&g_graph_relay, &graph_relay::netlist_event, this, &netlist_watcher::handle_netlist_event);
    connect(&g_graph_relay, &graph_relay::net_event, this, &netlist_watcher::handle_net_event);
    connect(&g_graph_relay, &graph_relay::module_event, this, &netlist_watcher::handle_module_event);

    reset();
}

netlist_watcher::~netlist_watcher()
{
}

void netlist_watcher::reset()
{
    m_notified = false;
}

void netlist_watcher::handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(object)
    Q_UNUSED(associated_data)

    handle_netlist_modified();
}

void netlist_watcher::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(object)
    Q_UNUSED(associated_data)

    handle_netlist_modified();
}

void netlist_watcher::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(object)
    Q_UNUSED(associated_data)

    handle_netlist_modified();
}

void netlist_watcher::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(object)
    Q_UNUSED(associated_data)

    handle_netlist_modified();
}

void netlist_watcher::handle_netlist_modified()
{
    if (!m_notified)
    {
        m_notified = true;
        g_content_manager.data_changed();
    }
}
