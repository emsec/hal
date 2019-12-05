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
class QTableWidget;
class QTableWidgetItem;

class module_details_widget : public QWidget
{
    Q_OBJECT
public:
    module_details_widget(QWidget* parent = nullptr);

    void update(u32 module_id);

public Q_SLOTS:

    void handle_selection_changed(void* sender);
    void handle_searchbar_text_edited(const QString &text);

    //relevant handler methods for net relay
    void handle_module_name_changed(const std::shared_ptr<module> m);
    void handle_module_gate_assigned(const std::shared_ptr<module> m, const u32 assigned_gate);
    void handle_module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate);

    void handle_gate_name_changed(const std::shared_ptr<gate> g);
    void handle_gate_removed(const std::shared_ptr<gate> g);

    void handle_net_removed(const std::shared_ptr<net> n);
    void handle_net_name_changed(const std::shared_ptr<net> n);
    void handle_net_src_changed(const std::shared_ptr<net> n);
    void handle_net_dst_added(const std::shared_ptr<net> n, const u32 dst_gate_id);
    void handle_net_dst_removed(const std::shared_ptr<net> n, const u32 dst_gate_id);

private:

    void toggle_searchbar();
    void handle_tree_double_clicked(const QModelIndex &index);
    void toggle_resize_columns();

    int compute_overall_number_of_gates();

    QVBoxLayout* m_content_layout;

    u32 m_current_id;
    bool m_ignore_selection_change;

    QTableWidget* m_general_table;
    QTableWidgetItem* m_name_item;
    QTableWidgetItem* m_id_item;
    QTableWidgetItem* m_gates_count_item;

    QTreeView* m_treeview;
    tree_module_model* m_tree_module_model;
    tree_module_proxy_model* m_tree_module_proxy_model;
};

#endif // MODULE_DETAILS_WIDGET_H
