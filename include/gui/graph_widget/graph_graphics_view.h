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

#ifndef GRAPH_GRAPHICS_VIEW_H
#define GRAPH_GRAPHICS_VIEW_H

#include <QGraphicsView>

class graph_widget;
class graph_graphics_item;

class graph_graphics_view : public QGraphicsView
{
    Q_OBJECT

public:
    graph_graphics_view(graph_widget* widget);

    //TODO REPLACE WITH RELAY
private Q_SLOTS:
    void handle_change_color_action();

private:
    virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent*) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

    void show_context_menu(const QPoint& pos);

    graph_widget* m_widget;

    //TODO REPLACE WITH RELAY
    graph_graphics_item* m_item;
};

#endif    // GRAPH_GRAPHICS_VIEW_H
