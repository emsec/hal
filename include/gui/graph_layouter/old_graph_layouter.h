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

#ifndef OLD_GRAPH_LAYOUTER_H
#define OLD_GRAPH_LAYOUTER_H

#include "graph_layouter/gui_graph_gate.h"
#include "graph_layouter/gui_graph_net.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/module.h"
#include "netlist_relay/netlist_relay.h"
#include <QGraphicsScene>
#include <QMap>
#include <QObject>
#include <QTime>
#include <vector>

class old_graph_layouter : public QObject
{
    Q_OBJECT
public:
    old_graph_layouter();
    ~old_graph_layouter();

    old_graph_layouter(QGraphicsScene* scene, std::shared_ptr<netlist> g);

    /** main function that layouts the graph */
    void layout_graph();

    void relayout_graph();

    /*functions for the nets*/
    gui_graph_gate* get_gui_gate_from_gate(std::shared_ptr<gate> gate);
    gui_graph_gate* get_gui_gate_from_id(int id);
    gui_graph_net* get_gui_graph_net_from_id(int id);

    /*returns the width of the channels*/
    int get_channal_width();

    /*returns the number of subchannels*/
    int get_net_sub_channels_count();

    /*returns the sub_channel_width*/
    int get_net_sub_channel_width();

    /*maybe start to compute the new layout after this?*/
    void set_channel_width(int new_channel_width);

    /*sets the number of subchannels*/
    void set_sub_channel_count(int new_count);

    /*sets the sub_channel_width*/
    void set_sub_channel_width(int new_width);

public Q_SLOTS:
    
    void handle_gate_name_changed_event(std::shared_ptr<gate> gate);
    
    void handle_module_gate_assigned(std::shared_ptr<module> module, u32 associated_data);
    
    void handle_module_gate_removed(std::shared_ptr<module> module, u32 associated_data);

Q_SIGNALS:
    /*maybe a parameter(id or something like that)*/
    void finished_layouting();

private:
    /*
     * member variables
     */
    //deleted in layouter-view
    QGraphicsScene* m_scene;

    std::shared_ptr<netlist> m_graph;

    bool m_nets_are_selected;

    /*save the highest level*/
    int highest_level = 0;

    /*stores the number of gates per level, needs to be removed*/
    std::vector<int> gates_per_level;

    /*the 2-dimensional array-logic*/
    std::vector<std::vector<gui_graph_gate*>> gate_layout;

    /*stores the current channel with which the nets are calculated*/
    int current_channel = 0;

    /*stores the width of each sub_channel, needed for computing the channel_width*/
    int sub_channel_width = 30;

    /*computed in get_channel_width*/
    int channel_width = 250;

    /*the vertical channel width, the nets need adjust the ents*/
    int channel_width_y = channel_width / 2;

    /*computed in get_channel_width*/
    int net_sub_channel_count = 20;

    /*goal: both in 1 datastruktur*/
    std::map<int, gui_graph_gate*> id_gui_gate_lookup;
    std::map<int, gui_graph_net*> id_gui_graph_net_lookup;

    //datastructures to be able to handle gate color history, maps the module id to its "previous" gate-set
    QMap<int, std::set<std::shared_ptr<gate>>> previous_module_gate_sets;

    std::map<int, int> id_level_lookup;    //gui_gate->level-lookup

    /*timer for debuging-purposes*/
    QTime global_timer;

    /*
     * private functions
     */

    void generate_gui_gates();

    /*computes the level of each gate using a BFS-like algorithm*/
    void compute_level_of_gates();

    /*puts the gui_gates in their computed places on the scene*/
    void apply_gate_level_placement();

    /*distributes the gates over all the levels, will most likely be called after get_level_of_gates*/
    void squarify();

    /*for placement/draw testing*/
    void draw_nets_with_net_algorithm();

    /*for now: calculates + draws the nets*/
    void draw_nets_as_straight_lines();    //draws the nets

    /*returns -1 if the gate has no level or ist not in the gate_level_assisgnment_vector, otherwise return the level*/
    int get_gate_level(gui_graph_gate* gate);

    /*returns the level of the gate*/
    int get_gate_level(int id);

    /*computes the maximal channel width (space between gates), needs to get the width each net line gets (for the selection)*/
    int compute_channel_width(int line_width);

    /*expands the scene rect with a fixed value*/
    void expand_scene_rect();

    void reset_state();
};

#endif    // OLD_GRAPH_LAYOUTER_H
