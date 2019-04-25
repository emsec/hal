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

#ifndef GRAPH_WIDGET_H
#define GRAPH_WIDGET_H

#include "content_widget/content_widget.h"
#include "graph_widget/graph_graphics_view.h"

#include <QLayout>
#include <QtWidgets>

class graph_layouter_selection_widget;

class graph_widget : public content_widget
{
    Q_OBJECT

public:
    explicit graph_widget(QWidget* parent = 0);

    virtual void setup_toolbar(toolbar* toolbarp) Q_DECL_OVERRIDE;

    void subscribe(QString layouter);

    graph_graphics_view* view() const;
    QString get_layouter();

    void show_view();

public Q_SLOTS:
    void zoom_in(int level = 1);
    void zoom_out(int level = 1);

private Q_SLOTS:
    void setup_matrix();
    void show_layout_selection(bool checked);
    //    void toggle_pointer_mode();
    void toggle_opengl();
    void toggle_antialiasing();

private:
    QStackedWidget* m_stacked_widget;
    graph_layouter_selection_widget* m_selection_widget;
    QWidget* m_view_container;
    graph_graphics_view* m_view;
    QToolButton* m_layout_button;
    //    QToolButton *m_select_mode_button;
    //    QToolButton *m_drag_mode_button;
    QToolButton* m_opengl_button;
    QToolButton* m_antialias_button;
    QSlider* m_zoom_slider;
    bool m_opengl_viewport;

    QString m_layouter;
};

#endif    // GRAPH_WIDGET_H
