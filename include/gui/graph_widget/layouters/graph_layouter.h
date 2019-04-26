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

#ifndef GRAPH_LAYOUTER_H
#define GRAPH_LAYOUTER_H

#include "graph_relay/graph_relay.h"
#include "netlist/gate.h"
#include "netlist/net.h"

#include <QObject>

class graphics_gate;
class graphics_net;
class graph_scene;

class graph_layouter : public QObject
{
    Q_OBJECT

public:
    explicit graph_layouter(QObject* parent = 0);
    virtual ~graph_layouter();

    virtual void layout(graph_scene* scene) = 0;
    virtual void reset()                    = 0;

    const QString& get_name() const;
    const QString& get_description() const;

public Q_SLOTS:
    virtual void handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> netlist, u32 associated_data)      = 0;
    virtual void handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)                  = 0;
    virtual void handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)                      = 0;
    virtual void handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data) = 0;

protected:
    QString m_name;
    QString m_description;

    graph_scene* m_scene;
};

#endif    // GRAPH_LAYOUTER_H
