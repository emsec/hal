//  MIT License
//
//  Copyright (c) 2019 Marc Fyrbiak
//  Copyright (c) 2019 Sebastian Wallat
//  Copyright (c) 2019 Max Hoffmann
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

#ifndef gui_graph_net_H
#define gui_graph_net_H

#include "graph_layouter/gui_graph_gate.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>

/*the different types of a net, the inout type is currently missing*/
enum class net_type
{
    global_input  = 0,
    global_output = 1,
    standard      = 2,
    vcc_net       = 3,
    gnd_net       = 4
};

/* class forwarding*/
class old_graph_layouter;
class gui_graph_graphics_item;

class gui_graph_net : public QObject, public gui_graph_graphics_item
{
    Q_OBJECT
public:
    gui_graph_net(QGraphicsItem* parent = 0);

    gui_graph_net(std::shared_ptr<net> net, QGraphicsItem* parent = 0);

    gui_graph_net(std::shared_ptr<net> net, QGraphicsScene* scene, old_graph_layouter* layouter, QGraphicsItem* parent = 0);

    /*
     * inherited functions
     */
    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    QPainterPath shape() const;

    void set_pen_width(int width);

    void open_edit_name_dialog();

    /*
     * specific gui_graph_net functions
     */

    /** calculated the layout of the net */
    void calculate_net(int sub_channel_a);

    /** returns its sub channel */
    int get_sub_channel();

    /** returns its internal stored net */
    std::shared_ptr<net> get_ref_net();

    void add_new_module_color(QColor color, int module_id);
    void remove_module_color(int module_id);

private:
    /*
     * private members
     */

    std::shared_ptr<net> refNet;

    //everything below already deleted in other instances
    old_graph_layouter* m_layouter;
    QGraphicsScene* m_scene;
    gui_graph_gate* src_gui_gate;

    std::set<gui_graph_gate*> dst_gui_gates;

    /*the net as a path containing many polygons*/
    QPainterPath path;
    QPolygonF pol;

    QList<QPair<QColor, int>> module_color_history;
    QColor default_color;

    /*the pen used for drawing with its width*/
    QPen m_pen;
    int pen_width = 2;

    QPainterPath subPaths;

    net_type type = net_type::standard;    //default

    QPointF src_pin;
    QFont vcc_gnd_font;
    QVector<QPointF> vcc_gnd_text_points;
    QString vcc_gnd_text;

    /*unmapped scene positions and mapped scene positions of the destianation pins*/
    std::vector<QPointF> dst_pins;
    std::vector<QPointF> dst_pins_mapped;

    /* the subchannel of the net with its with (needs to be the same as in the graph_layouter) */
    int sub_channel = 0;
    int line_width  = 16;

    /*
     * private functions
     */

    /* executes the actual "algorithm" */
    void compute_standard_net_v2(int line);

    /* computes the global input and output net*/
    void compute_global_input_net();
    void compute_global_output_net();

    /*computes the gnd and the vcc net*/
    void compute_vcc_or_gnd_net();
};

#endif    // gui_graph_net_H
