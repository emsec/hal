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

#ifndef MODULE_DETAILS_WIDGET_H
#define MODULE_DETAILS_WIDGET_H

#include "def.h"

#include "netlist_relay/netlist_relay.h"

#include <QWidget>
#include <QTreeView>
#include "selection_details_widget/tree_navigation/tree_module_model.h"
#include "selection_details_widget/tree_navigation/tree_module_proxy_model.h"

class module;

class QVBoxLayout;
class QLabel;
class searchbar;

class module_details_widget : public QWidget
{
    Q_OBJECT
public:
    module_details_widget(QWidget* parent = nullptr);

    void update(u32 module_id);

public Q_SLOTS:
    void handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data);

private:

    void toggle_searchbar();
    void handle_searchbar_text_edited(const QString &text);

    void toggle_resize_columns();

    QVBoxLayout* m_content_layout;

    u32 m_current_id;

    QTreeView* m_treeview;
    tree_module_model* m_tree_module_model;
    tree_module_proxy_model* m_tree_module_proxy_model;

    searchbar* m_searchbar;
};

#endif // MODULE_DETAILS_WIDGET_H
