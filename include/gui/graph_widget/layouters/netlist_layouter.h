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

#ifndef NETLIST_LAYOUTER_H
#define NETLIST_LAYOUTER_H

#include "graph_widget/graphics_items/graphics_gate.h"
#include "graph_widget/layouters/graph_layouter.h"
#include "netlist/gate.h"
#include "netlist/net.h"

#include <QList>
#include <QMap>

class netlist_layouter : public graph_layouter
{
    Q_OBJECT

public:
    netlist_layouter();

    virtual void layout(graph_scene* scene);
    virtual void reset();

public Q_SLOTS:
    virtual void handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> netlist, u32 associated_data);
    virtual void handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data);
    virtual void handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data);
    virtual void handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data);

private:
    //###Layout-Settings Area [these variables will be set through the settings]
    bool m_squarify_entire_design;
    bool m_compress_gate_levels;
    bool m_compute_cw_scc;
    int m_compress_gate_levels_value;
    int m_sub_channel_count;
    int m_sub_channel_width;
    int m_channel_width_x;
    qreal m_x_y_channel_width_ratio;
    int m_channel_width_y;
    //###End of Layout-Settings Area
    int m_current_sub_channel;

    //The only three (self-explaining) datastructures that are needed to handle the layouting
    QMap<std::shared_ptr<gate>, graphics_gate*> m_gate_to_gui_gate_map;
    QMap<std::shared_ptr<gate>, int> m_gate_to_level;
    QVector<QVector<graphics_gate*>> m_layout;

    qreal m_max_gate_width;
    qreal m_max_gate_height;

    qreal m_min_gate_width;
    qreal m_min_gate_height;

    //All Layout substeps and additional steps
    //Gate-specific functions
    void generate_gui_gates();
    void compute_gate_levels();
    void apply_gate_layout();
    void squarify_entire_design();
    void compress_gate_levels();
    void compute_cw_and_scc();

    //Net-specific functions
    //Facade-pattern, less overall steps and the nets need to be drawn in a specific order
    void draw_nets();
    void draw_input_net(std::shared_ptr<net> n);
    void draw_output_net(std::shared_ptr<net> n);
    void draw_standard_net(std::shared_ptr<net> n);
    void draw_separated_net(std::shared_ptr<net> n, QString text);

    //For better understanding and overview (this can be a public function as well)
    void init_settings();
};
#endif    // NETLIST_LAYOUTER_H
