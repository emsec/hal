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

#ifndef GRAPH_GRAPHICS_WIDGET_H
#define GRAPH_GRAPHICS_WIDGET_H

//#include "graph_graphics_view.h"
#include "graph_layouter/graph_layouter_view.h"
#include "graph_layouter/gui_graph_gate.h"
#include "graph_layouter/gui_graph_net.h"
#include "graph_manager/graph_background_scene.h"
#include <QGraphicsView>
#include <QLayout>
#include <QObject>
#include <QTabWidget>
#include <QWidget>

class graph_graphics_widget : public QWidget
{
    Q_OBJECT

public:
    graph_graphics_widget(QGraphicsView* view);

public Q_SLOTS:

    void resize_background();

private Q_SLOTS:

    void ShowContextMenu(const QPoint& pos);

    void edit_gate_data();
    void edit_net_data();

    //2 slots are needed, one to connect the context-menu with the widget, the other to connect the result of the widget back to this class
    void handle_add_gates_to_new_module_triggered();
    void handle_add_gates_to_existing_module();
    void handle_remove_gates_from_module_triggered();

private:
    QGraphicsView* m_view;

    graph_background_scene m_background_scene;

    QGraphicsView m_background;

    QGridLayout m_layout;

    QGridLayout m_background_layout;

    //    gate_ui *m_context_menu_gate;

    //hotfix for now, if you compute the gui_graph_gates with parallel with openmp in the layouter, you cant properly connect
    //the contextmenu actions to the gate(it fails to register the nodes to the metadata?), so temporarily use the slots of this class
    //and save the current node
    gui_graph_gate* current_context_gate;
    gui_graph_net* current_context_net;
    QList<gui_graph_gate*> currently_selected_gates;
    QList<gui_graph_net*> currently_selected_nets;
};

#endif    // GRAPH_GRAPHICS_WIDGET_H
