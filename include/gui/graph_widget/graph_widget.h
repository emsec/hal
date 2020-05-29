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

#include "gui/gui_globals.h"
#include "gui/gui_def.h"
#include "gui/content_widget/content_widget.h"
#include "gui/graph_widget/contexts/graph_context_subscriber.h"

#include <deque>

class dialog_overlay;
class graph_context;
class graph_graphics_view;
class graph_layout_spinner_widget;
class graph_navigation_widget;

class graph_widget : public content_widget, public graph_context_subscriber
{
    Q_OBJECT

public:
    explicit graph_widget(graph_context* context, QWidget* parent = nullptr);

    graph_context* get_context() const;

    void handle_scene_available() override;
    void handle_scene_unavailable() override;
    void handle_context_about_to_be_deleted() override;

    void handle_status_update(const int percent) override;
    void handle_status_update(const QString& message) override;

    graph_graphics_view* view();

    void ensure_selection_visible();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private Q_SLOTS:
    void handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
    void handle_module_double_clicked(const u32 id);
    void reset_focus();

private:
    void handle_navigation_left_request();
    void handle_navigation_right_request();
    void handle_navigation_up_request();
    void handle_navigation_down_request();

    void substitute_by_visible_modules(const QSet<u32>& gates, const QSet<u32>& modules, QSet<u32>& insert_gates, QSet<u32>& insert_modules,
                                       QSet<u32>& remove_gates, QSet<u32>& remove_modules) const;
    void set_modified_if_module();

    void handle_enter_module_requested(const u32 id);

    void ensure_items_visible(const QSet<u32>& gates, const QSet<u32>& modules);

    graph_graphics_view* m_view;
    graph_context* m_context;

    dialog_overlay* m_overlay;
    graph_navigation_widget* m_navigation_widget;
    graph_layout_spinner_widget* m_spinner_widget;

    u32 m_current_expansion;
};

#endif // GRAPH_WIDGET_H
