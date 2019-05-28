#ifndef NETLIST_WATCHER_H
#define NETLIST_WATCHER_H

#include "graph_relay/graph_relay.h"

#include <QObject>

class netlist_watcher : public QObject
{
    Q_OBJECT
public:
    netlist_watcher(QObject* parent = 0);
    ~netlist_watcher();

    bool has_netlist_unsaved_changes();
    void set_netlist_unsaved_changes(bool has_netlist_unsaved_changes);

public Q_SLOTS:
    void handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data);
    void handle_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data);
    void handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data);
    void handle_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data);

private:
    void handle_netlist_modified();

    bool m_has_netlist_unsaved_changes;
};

#endif // NETLIST_WATCHER_H
