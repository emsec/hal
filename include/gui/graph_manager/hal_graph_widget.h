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

#ifndef HAL_GRAPH_WIDGET_H
#define HAL_GRAPH_WIDGET_H

#include "content_widget/content_widget.h"
#include "graph_manager/graph_graphics_widget.h"
#include "selection_history_navigator/selection_history_navigator.h"

#include <QPushButton>

class hal_graph_widget : public content_widget
{
    Q_OBJECT

public:
    hal_graph_widget(QGraphicsView* view);

    virtual void setup_toolbar(toolbar* toolbar);

public Q_SLOTS:
    void handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data);
    void handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data);

Q_SIGNALS:
    void relayout_button_clicked();

private:
    graph_graphics_widget m_graphics_widget;

    void handle_relayout_button_clicked();
    void handle_back_activated();
    void handle_next_activated();

    QPushButton* m_relayout_button;

    selection_history_navigator* m_selection_history_navigator;
};

#endif    // HAL_GRAPH_WIDGET_H
