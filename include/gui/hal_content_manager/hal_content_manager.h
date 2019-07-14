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

#ifndef HAL_CONTENT_MANAGER_H
#define HAL_CONTENT_MANAGER_H

#include <QGraphicsScene>
#include <QObject>
#include <QStringList>

//#include "gui/graph_manager/hal_graph_widget.h"
#include "gui/graph_layouter/old_graph_layouter.h"
#include "netlist_watcher/netlist_watcher.h"

class main_window;

class hal_tab_widget;
class file_manager;
class content_widget;
class console_widget;
class python_editor;
class python_console_widget;
class old_graph_layouter;
class graph_layouter_view;

class hal_content_manager : public QObject
{
    Q_OBJECT

public:
    explicit hal_content_manager(main_window* parent);

    ~hal_content_manager();
    
    void hack_delete_content();

Q_SIGNALS:
    void save_triggered();

public Q_SLOTS:

    void handle_open_document(const QString& file_name);

    void handle_close_document();

    void handle_filsystem_doc_changed(const QString& file_name);

    void handle_relayout_button_clicked();

    void handle_save_triggered();

private:
    main_window* m_main_window;

    QString m_window_title;

    QList<content_widget*> m_content;

    //More testing purposes, delete them later on and put all in m_content
    QList<content_widget*> m_content2;

    //temporary solution to make the view a member variable(all this stuff might be deleted later nonetheless)
    //graph_layouter_view* m_layouter_view;

    //temporary solution aslong as the involved classes are strongly coupled
    QGraphicsScene* m_graph_scene;

    graph_layouter_view* m_layouter_view;

    old_graph_layouter* layouter;

    python_editor* m_python_widget;

    netlist_watcher* m_netlist_watcher;

    std::set<QString> m_unsaved_changes;
};

#endif    // HAL_CONTENT_MANAGER_H
