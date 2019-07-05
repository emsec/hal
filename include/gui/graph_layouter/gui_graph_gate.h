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

#ifndef gui_graph_gate_H
#define gui_graph_gate_H

#include "gui_graph_graphics_item.h"
#include "netlist/gate.h"
#include <QColor>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QList>
#include <QObject>
#include <QPair>
#include <QPoint>

struct pin
{
    int x;
    int y;
    QString name;
    QString type;
};
//you need to inherit from qobject as well, because qgraphicsitem doesnt inherit from qobject, but you have to in order to use signals/slots
class gui_graph_gate : public QObject, public gui_graph_graphics_item
{
    Q_OBJECT
public:
    gui_graph_gate(QGraphicsItem* parent = 0);

    gui_graph_gate(std::shared_ptr<gate> gate, QGraphicsItem* parent = 0);

    /*
     * inherited functions
     */

    /*the boundingRect surrounds the pins too, so the width is textwidth + 2x pinlength + buffer*/
    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

    void set_pen_width(int new_width);

    /*
     * some gui_gate specific functions
     */

    /** static function that returns the height of the heighest gate */
    static int get_max_height();

    /** static functions that returns the width of the biggest gate */
    static int get_max_width();

    /** static functions that returns the height of the smalles gate */
    static int get_min_height();

    /** static function that returns the width of the smalles gate */
    static int get_min_widht();

    /** resets the static variables width and heigth to its initialization values */
    static void reset_height_and_width();

    /** returns the width of the gate */
    int get_width();

    /** returns the height of the gate */
    int get_height();

    /** returns the input pin type at the given position, used for the graph navigation */
    std::string get_input_pin_type_at_position(unsigned int pos);

    /** returns the output pin type at the given position, used for the graph navigation */
    std::string get_output_pin_type_at_position(unsigned int pos);
    
    //adds the offset to the pin coordinates, so the layouter has less work
    /** returns the unmapped input pin coordinates for a given type */
    QPointF get_unmapped_input_pin_coordinates(QString type);

    /** returns the unmapped output pin coordinates for a given type */
    QPointF get_unmapped_output_pin_coordinates(QString type);

    /** returns the mapped input pin coordinates for a given type */
    QPointF get_mapped_input_pin_coordinates(QString type);

    /** returns the mapped output pin coordinates for a given type */
    QPointF get_mapped_output_pin_coordinates(QString type);

    /** returns the internal stored graph gate */
    std::shared_ptr<gate> get_ref_gate();

    //functions that may be needed for (un-)unifying gates, not needed for now
    /** sets the new layout position */
    void set_layout_position(QPoint new_position);

    /** sets the new layout position */
    void set_layout_position(int position_x, int position_y);

    /** returns the new layout position */
    QPoint get_layout_position();

    //for navigation purpose (only left or right pin navigation, not both, need to check first)
    /** enables output pin navigation */
    void enable_output_pin_navigation(bool activate);

    /** enables input pin navigation */
    void enable_input_pin_navigation(bool activate);

    /** increases the current pin position */
    void increase_current_pin();

    /** decreases the current pin position */
    void decrease_current_pin();

    /** returns the currently selected pin type */
    std::string get_current_pin_type();    //returns -1 if no pin is selected, maybe make a tuple with the infos wether its an input-pin or output-pin

    /** returns the number of input pins */
    int input_pin_count();

    /** returns the number of utput pins*/
    int output_pin_count();

    void update_name();

public Q_SLOTS:
    //this function is for the contextmenu, so the graphic_widget doesnt need to safe the gate as a member(this functions open a qcolordialog)
    void change_color();
    void set_color(QColor color);
    //adds a new color to the history with the correspondig module id
    void add_new_module_color(QColor color, int id);
    //removes the color that corresponds the module id
    void remove_module_color(int module_id);
    //a function for the same reason stated above
    void open_edit_name_dialog();

private:
    /*
     * private member
     */

    /*holds the reference of its gate*/
    std::shared_ptr<gate> m_ref_gate;

    /*may be usefull to have*/
    QGraphicsTextItem* name;
    QString text_gate_name;
    QFont name_font;
    QFont pin_font;
    QColor defaultColor;
    QColor gate_color;
    QString text_gate_name_without_type;
    //the 0-index is always the default color(or check if the length is 0, then take default color), should not be removed
    QList<QPair<QColor, int>> module_color_history;

    /*height/width of the boundingRect*/
    int m_width;
    int height;

    /*saves the layout-position, maybe needed for unifying gates to one big gate (un-unifying)*/
    QPoint layout_position;

    /*some constants for the gate*/
    int drawn_pin_height      = 10;
    int pin_width             = 15;
    int input_pin_name_width  = 35;
    int output_pin_name_width = 20;
    int overall_pin_height    = 2 * drawn_pin_height;
    int pen_width             = 2;
    int number_output_pins;
    int number_input_pins;

    /*the required paint-onjects*/
    QRect name_rect;
    QRect input_pin_name_rect;
    QRect output_pin_name_rect;
    QRect drawn_outer_rect;

    /*saves the pins with its needed infos, may be changed to a map*/
    std::vector<pin> input_pins;
    std::vector<pin> output_pins;

    /*stores the input pins that have a gnd/vcc net as input*/
    std::vector<pin> input_pins_gnd;
    std::vector<pin> input_pins_vcc;

    /*saves the maximum width and height of all gui_gates*/
    static int MAX_HEIGHT;
    static int MAX_WIDTH;

    /*saves the minimum width and height of all gui_gates*/
    static int MIN_HEIGHT;
    static int MIN_WIDTH;

    //variables needed for pin navigation;
    bool output_pin_navogation_on = false;
    bool input_pin_navigation_on  = false;
    int input_pin_counter         = 0;
    int output_pin_counter        = 0;

    /*
     * private functions
     */

    /*functions to split the paint method into sweet little pieces*/
    void paint_output_pins(QPainter* painter);
    void paint_input_pins(QPainter* painter);
    void draw_module_color_squares(QPainter* painter);
};

#endif    // gui_graph_gate_H
