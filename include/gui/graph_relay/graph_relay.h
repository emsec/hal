//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#ifndef GRAPH_RELAY_H
#define GRAPH_RELAY_H

#include <QObject>

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"
#include "netlist/event_system/module_event_handler.h"

class graph_relay : public QObject
{
    Q_OBJECT

public:
    explicit graph_relay(QObject* parent = 0);
    ~graph_relay();

    void register_callbacks();

    void relay_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data);

    void relay_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data);

    void relay_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data);

    void relay_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data);

Q_SIGNALS:
    void netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data);
    void net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data);
    void gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data);
    void module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data);
};

#endif    // GRAPH_RELAY_H
