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

#include "def.h"

#include "gui/content_widget/content_widget.h"
#include "gui/graph_widget/contexts/graph_context_subscriber.h"

class dialog_overlay;
class cone_layouter;
class graph_context;
class graph_graphics_view;
class graph_layout_progress_widget;
class graph_layout_spinner_widget;
class graph_navigation_widget;

class graph_widget : public content_widget, public graph_context_subscriber
{
    Q_OBJECT

public:
    graph_widget(QWidget* parent = nullptr);

    virtual void setup_toolbar(toolbar* toolbar) Q_DECL_OVERRIDE;

    virtual void handle_scene_available() Q_DECL_OVERRIDE;
    virtual void handle_scene_unavailable() Q_DECL_OVERRIDE;
    virtual void handle_context_about_to_be_deleted() Q_DECL_OVERRIDE;

    virtual void handle_status_update(const int percent) Q_DECL_OVERRIDE;
    virtual void handle_status_update(const QString& message) Q_DECL_OVERRIDE;

protected:
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void handle_navigation_jump_requested(const u32 from_gate, const u32 via_net, const u32 to_gate);
    void handle_module_double_clicked(const u32 id);
    void reset_focus();

private:
    void handle_navigation_left_request();
    void handle_navigation_right_request();
    void handle_navigation_up_request();
    void handle_navigation_down_request();

    void handle_module_up_request();
    void handle_module_down_requested(const u32 id);

    void debug_module_one();
    void debug_create_context();
    void debug_change_context();

    void change_context(graph_context* const context);

    void handle_updating_scene();
    //void handle_scene_available();

    void ensure_gate_visible(const u32 gate);

    graph_graphics_view* m_view;
    graph_context* m_context;

    dialog_overlay* m_overlay;
    graph_navigation_widget* m_navigation_widget;
    graph_layout_progress_widget* m_progress_widget;
    graph_layout_spinner_widget* m_spinner_widget;

    u32 m_current_expansion;
};

#endif // GRAPH_WIDGET_H
