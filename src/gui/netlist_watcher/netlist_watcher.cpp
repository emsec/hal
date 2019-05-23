#include "netlist_watcher/netlist_watcher.h"

#include "gui_globals.h"

netlist_watcher::netlist_watcher(QObject* parent) : QObject(parent)
{
    m_has_netlist_unsaved_changes = false;

    connect(&g_graph_relay, &graph_relay::gate_event, this, &netlist_watcher::handle_gate_event);
    connect(&g_graph_relay, &graph_relay::netlist_event, this, &netlist_watcher::handle_netlist_event);
    connect(&g_graph_relay, &graph_relay::net_event, this, &netlist_watcher::handle_net_event);
    connect(&g_graph_relay, &graph_relay::module_event, this, &netlist_watcher::handle_module_event);
}

netlist_watcher::~netlist_watcher(){}

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
    m_has_netlist_unsaved_changes = true;
}

bool netlist_watcher::has_netlist_unsaved_changes()
{   
    return m_has_netlist_unsaved_changes;
}

void netlist_watcher::set_netlist_unsaved_changes(bool has_netlist_unsaved_changes)
{
    m_has_netlist_unsaved_changes = has_netlist_unsaved_changes;
}
