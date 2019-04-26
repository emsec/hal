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

#ifndef GRAPH_LAYOUTER_VIEW_H
#define GRAPH_LAYOUTER_VIEW_H

#include "graph_layouter/gui_graph_gate.h"
#include "graph_layouter/gui_graph_net.h"
#include "graph_layouter/old_graph_layouter.h"
#include "netlist/gate.h"
#include "netlist/netlist.h"
#include "selection_relay/selection_relay.h"
#include <QAction>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QPoint>
#include <QRectF>
#include <QScrollBar>
#include <QStack>
#include <QWidget>
#include <qmath.h>

class graph_layouter_view : public QGraphicsView
{
    Q_OBJECT

public:
    graph_layouter_view(QGraphicsScene* scene, old_graph_layouter* layouter, std::shared_ptr<netlist> g);
    ~graph_layouter_view();

    void gentle_zoom(double factor);

    void set_modifiers(Qt::KeyboardModifiers modifiers);

    void set_zoom_factor_base(double value);

    void paintEvent(QPaintEvent* event);

    void keyPressEvent(QKeyEvent* event);

    void keyReleaseEvent(QKeyEvent* event);

    void resizeEvent(QResizeEvent* event);

    QList<gui_graph_gate*> get_selected_gates();
    QList<gui_graph_net*> get_selected_nets();

public Q_SLOTS:
    void onSelectionChanged();
    void onSuccessorgateSelected(std::shared_ptr<gate> successor);
    void onShortcutActivated();
    void zoom_in();
    void zoom_out();
    void set_layouter(old_graph_layouter* layouter);
    void handle_graph_relayouted();

    /*
     * following functions handle the selection relay signals
     */
    void handle_current_gate_update(void* sender, u32 id);
    void handle_current_net_update(void* sender, u32 id);
    void handle_combined_selection_update(void* sender, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids, selection_relay::Mode mode);

private:
    /*
     * member variables
     */

    old_graph_layouter* m_layouter;
    std::shared_ptr<netlist> m_netlist;

    Qt::KeyboardModifiers _modifiers;

    double _zoom_factor_base;
    double current_zoom_factor = 1.;

    /* following variables handle the limits of the new zooming method*/
    double original_view_scene_ratio;
    double entire_graph_view_scene_ratio;
    double current_view_scene_ratio;

    /*these handle the zooming*/
    int overall_number_of_zoom_level;
    double zoom_in_factor  = 1.27094;
    double zoom_out_factor = 0.786816;
    int zoom_level         = 0;

    /*current pen_width of the graphicsitems*/
    int pen_width = 2;

    /*currently disabled*/
    bool zoom_activated = false;

    /*rects needed for zooming purposes*/
    QPointF target_scene_pos, target_viewport_pos;
    QRectF defaultRect, startingRect;

    /*the enum and enum-variable are for navigation purpose only*/
    enum internal_navigator_state
    {
        no_gate_selected          = 0,
        gate_selected             = 1,
        gate_output_pins_selected = 2,
        gate_input_pins_selected  = 3,
        net_selected              = 4
    };

    /*stores the internal state state that is used for the navigation*/
    internal_navigator_state state = no_gate_selected;

    /*these variable store the currently selected gate and/or net*/
    gui_graph_gate* currently_selected_gate = nullptr;
    gui_graph_net* currently_selected_net   = nullptr;

    /*some variables needed for onselectionchanged method and resizeeven*/
    bool first_time_focus = false;
    bool just_handle_call = false;

    /*stores the currently selected gates and nets*/
    QList<gui_graph_gate*> selected_gates;
    QList<gui_graph_net*> selected_nets;

    QStack<gui_graph_gate*> gate_history;

    /*
     * some private functions
     */

    /*sets the pen_width of each graphicsitem (gates and nets)*/
    void set_graphicsitems_pen_width(int width);

    bool eventFilter(QObject* object, QEvent* event);

    /*functions to split the keypressevent function in small pieces*/
    void gate_selected_key_pressed(QKeyEvent* event, gui_graph_gate* gate);
    void gate_left_pins_selected_key_pressed(QKeyEvent* event, gui_graph_gate* gate);
    void gate_right_pins_selected_key_pressed(QKeyEvent* event, gui_graph_gate* gate);
    void net_selected_key_pressed(QKeyEvent* event);

Q_SIGNALS:

    void zoomed();

    void redrawn();
};

#endif    // GRAPH_LAYOUTER_VIEW_H
