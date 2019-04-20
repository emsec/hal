#include "graph_relay/graph_relay.h"
#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include <functional>

#include <QDebug>

graph_relay::graph_relay(QObject* parent) : QObject(parent)
{
    register_callbacks();
}

graph_relay::~graph_relay()
{
    netlist_event_handler::unregister_callback("relay");
    net_event_handler::unregister_callback("relay");
    gate_event_handler::unregister_callback("relay");
    module_event_handler::unregister_callback("relay");
}

void graph_relay::register_callbacks()
{
    netlist_event_handler::register_callback("relay",
                                             std::function<void(netlist_event_handler::event, std::shared_ptr<netlist>, u32)>(
                                                 std::bind(&graph_relay::relay_netlist_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    net_event_handler::register_callback(
        "relay",
        std::function<void(net_event_handler::event, std::shared_ptr<net>, u32)>(std::bind(&graph_relay::relay_net_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    gate_event_handler::register_callback("relay",
                                          std::function<void(gate_event_handler::event, std::shared_ptr<gate>, u32)>(
                                              std::bind(&graph_relay::relay_gate_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    module_event_handler::register_callback("relay",
                                               std::function<void(module_event_handler::event, std::shared_ptr<module>, u32)>(
                                                   std::bind(&graph_relay::relay_module_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void graph_relay::relay_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data)
{
    Q_EMIT netlist_event(ev, object, associated_data);
    //qDebug() << "relay_netlist_event called: event ID =" << ev << "for object at" << object.get();
}

void graph_relay::relay_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data)
{
    Q_EMIT net_event(ev, object, associated_data);
    //qDebug() << "relay_net_event called: event ID =" << ev << "for object at" << object.get();
}

void graph_relay::relay_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data)
{
    Q_EMIT gate_event(ev, object, associated_data);
    //qDebug() << "relay_gate_event called: event ID =" << ev << "for object at" << object.get();
}

void graph_relay::relay_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data)
{
    Q_EMIT module_event(ev, object, associated_data);
    //qDebug() << "relay_module_event called: event ID =" << ev << "for object at" << object.get();
}
