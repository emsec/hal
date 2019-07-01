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

#ifndef NET_DETAILS_WIDGET_H
#define NET_DETAILS_WIDGET_H

#include "def.h"

#include "netlist_relay/netlist_relay.h"

#include <QWidget>

class QVBoxLayout;
class QLabel;
class QScrollArea;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class QTableWidgetItem;
class net;

class net_details_widget : public QWidget
{
    Q_OBJECT

public:
    net_details_widget(QWidget* parent = 0);
    ~net_details_widget();

    void update(u32 net_id);

public Q_SLOTS:
    void handle_tree_size_change(QTreeWidgetItem* item);
    void handle_item_expanded(QTreeWidgetItem* item);
    void handle_item_collapsed(QTreeWidgetItem* item);

    void handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data);

private:
    // NEW !!!!!

    QVBoxLayout* m_content_layout;

    QTableWidget* m_general_table;
    QTableWidgetItem* m_name_item;
    QTableWidgetItem* m_type_item;
    QTableWidgetItem* m_id_item;
    QTableWidgetItem* m_module_item;

    // stores input pin tree view
    QTreeWidgetItem* m_src_pin;
    // stores output pin tree view
    QTreeWidgetItem* m_dst_pins;

    // stores utility objects for input/output pin tree view
    QScrollArea* m_scroll_area;
    QTreeWidget* m_tree_widget;
    QVBoxLayout* m_scroll_area_layout;
    QVBoxLayout* m_container_layout;
    QWidget* m_container;

    QLabel* m_item_deleted_label;

    // NEW !!!!!;
    QLabel* m_label;
    QTableWidget* m_table_widget;

    int m_tree_height;
    int m_table_height;
    int m_spacing = 1;

    u32 m_current_id;

    QTreeWidgetItem* m_src_item;
    QTreeWidgetItem* m_dst_item;
};

#endif    // NET_DETAILS_WIDGET_H
