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

#include <QGraphicsScene>
#include <QObject>
#include <QStringList>

#include "netlist_watcher/netlist_watcher.h"

namespace hal
{
    class main_window;
    class content_widget;
    class python_editor;
    class graph_tab_widget;
    class old_graph_navigation_widget;
    class context_manager_widget;
    class netlist_watcher;

    class hal_content_manager : public QObject
    {
        Q_OBJECT

    public:
        explicit hal_content_manager(main_window* parent);

        ~hal_content_manager();

        python_editor* get_python_editor_widget();

        graph_tab_widget* get_graph_tab_widget();

        context_manager_widget* get_context_manager_widget();

        void hack_delete_content();

    Q_SIGNALS:
        void save_triggered();

    public Q_SLOTS:

        void handle_open_document(const QString& file_name);

        void handle_close_document();

        void handle_filsystem_doc_changed(const QString& file_name);

        void handle_save_triggered();

    private:
        main_window* m_main_window;

        QString m_window_title;

        QList<content_widget*> m_content;

        netlist_watcher* m_netlist_watcher;

        python_editor* m_python_widget;

        graph_tab_widget* m_graph_tab_wid;

        context_manager_widget* m_context_manager_wid;
    };
}
