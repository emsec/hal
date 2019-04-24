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

#ifndef gui_graph_gate_SELECTION_WIDGET_H
#define gui_graph_gate_SELECTION_WIDGET_H

#include "netlist/gate.h"
#include "netlist/netlist.h"
#include <QDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QSize>
#include <QTableWidget>
#include <QWidget>

class graph_layouter_view;
class gui_graph_gate_selection_widget : public QTableWidget
{
    Q_OBJECT

public:
    explicit gui_graph_gate_selection_widget(QWidget* parent, std::vector<endpoint> successors, std::shared_ptr<gate> curr_gate, std::string curr_pin_type);

    void keyPressEvent(QKeyEvent* event);

    //ONLY A TEMPORARY FUNCTION, WILL BE REMOVED AFTER REWORK
    void set_is_net_selected(bool b);

Q_SIGNALS:
    void gateSelected(std::shared_ptr<gate> selected);

private:
    /*
     * private members
     */
    //QTableWidget* m_table;
    //std::shared_ptr<netlist> m_netlist;
    graph_layouter_view* m_view;
    std::shared_ptr<gate> m_curr_gate;
    std::string m_curr_pin;
    bool is_net_selected = false;

    /*
     * private functions
     */

    /*calculated the "biggest" size of the table*/
    QSize table_widget_size();

    void on_item_double_clicked(QTableWidgetItem* item);
};

#endif    // gui_graph_gate_SELECTION_WIDGET_H
