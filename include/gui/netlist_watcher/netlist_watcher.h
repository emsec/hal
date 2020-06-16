#ifndef NETLIST_WATCHER_H
#define NETLIST_WATCHER_H

#include "netlist_relay/netlist_relay.h"

#include <QObject>

class netlist_watcher : public QObject
{
    Q_OBJECT
public:
    netlist_watcher(QObject* parent = 0);
    ~netlist_watcher();

    void reset();

private:
    void handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<Netlist> object, u32 associated_data);
    void handle_module_event(module_event_handler::event ev, std::shared_ptr<Module> object, u32 associated_data);
    void handle_gate_event(gate_event_handler::event ev, std::shared_ptr<Gate> object, u32 associated_data);
    void handle_net_event(net_event_handler::event ev, std::shared_ptr<Net> object, u32 associated_data);

    void handle_netlist_modified();
    bool m_notified;
};

#endif // NETLIST_WATCHER_H
