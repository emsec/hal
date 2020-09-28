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

#pragma once

#include "gui/content_widget/content_widget.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_widget.h"

#include <QMap>

class QTabWidget;
class QVBoxLayout;

namespace hal
{
    class GraphTabWidget : public ContentWidget
    {
        Q_OBJECT

    public:
        GraphTabWidget(QWidget* parent = nullptr);

        virtual QList<QShortcut*> create_shortcuts() override;

        int addTab(QWidget* tab, QString tab_name = "default");
        void show_context(GraphContext* context);

    public Q_SLOTS:
        void handle_context_created(GraphContext* context);
        void handle_context_renamed(GraphContext* context);
        void handle_context_removed(GraphContext* context);

        void handle_tab_changed(int index);

    private:
        QTabWidget* m_tab_widget;
        QVBoxLayout* m_layout;

        float m_zoom_factor;

        QMap<GraphContext*, QWidget*> m_context_widget_map;

        GraphWidget* m_current_widget;

        int get_context_tab_index(GraphContext* context) const;

        //functions
        void handle_tab_close_requested(int index);

        void add_graph_widget_tab(GraphContext* context);

        void zoom_in_shortcut();
        void zoom_out_shortcut();
    };
}
