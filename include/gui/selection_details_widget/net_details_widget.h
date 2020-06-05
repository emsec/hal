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
class QHBoxLayout;
class QLabel;
class QScrollArea;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QFont;
class net;
class gate;

class net_details_widget : public QWidget
{
    Q_OBJECT

public:
    net_details_widget(QWidget* parent = 0);
    ~net_details_widget();

    virtual bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void update(u32 net_id);

public Q_SLOTS:
    void handle_item_expanded(QTreeWidgetItem* item);
    void handle_item_collapsed(QTreeWidgetItem* item);

    void on_treewidget_item_clicked(QTreeWidgetItem* item, int column);

    void handle_net_removed(const std::shared_ptr<net> n);
    void handle_net_name_changed(const std::shared_ptr<net> n);
    void handle_net_source_added(const std::shared_ptr<net> n, const u32 src_gate_id);
    void handle_net_source_removed(const std::shared_ptr<net> n, const u32 src_gate_id);
    void handle_net_destination_added(const std::shared_ptr<net> n, const u32 dst_gate_id);
    void handle_net_destination_removed(const std::shared_ptr<net> n, const u32 dst_gate_id);

    void handle_gate_name_changed(const std::shared_ptr<gate> g);

private:

    //new design

    //general
    int m_current_id;
    QFont m_key_font;

    //utility objects to encapsulate the sections together to make it scrollable
    QScrollArea* m_scroll_area;
    QWidget* m_top_lvl_container;
    QVBoxLayout* m_top_lvl_layout;
    QVBoxLayout* m_content_layout;

    //buttons to fold/unfold the corresponding sections
    QPushButton* m_general_info_button;
    QPushButton* m_source_pins_button;
    QPushButton* m_destination_pins_button;

    //the sections to unfold

    //(1) general information section
    QTableWidget* m_general_table;
    QTableWidgetItem* m_name_item;
    QTableWidgetItem* m_type_item;
    QTableWidgetItem* m_id_item;
    QTableWidgetItem* m_module_item;

    //(2) source_pins section
    QTableWidget* m_source_pins_table;

    //(3) destination_pins section
    QTableWidget* m_destination_pins_table;

    //function section
    void handle_buttons_clicked();
    void handle_table_item_clicked(QTableWidgetItem* item);

    //utility function, used to calculate the actual width so the scrollbars and the accuracy of the click functionality is correct
    QSize calculate_table_size(QTableWidget* table);

    //end of new design

    // NEW !!!!!

//    QVBoxLayout* m_content_layout;
//    QHBoxLayout* m_tree_row_layout;

//    QTableWidget* m_general_table;
//    QTableWidgetItem* m_name_item;
//    QTableWidgetItem* m_type_item;
//    QTableWidgetItem* m_id_item;
//    QTableWidgetItem* m_module_item;

//    // stores input pin tree view
//    QTreeWidgetItem* m_source_pin;
//    // stores output pin tree view
//    QTreeWidgetItem* m_destination_pins;

//    // stores utility objects for input/output pin tree view
//    QScrollArea* m_scroll_area;
//    QTreeWidget* m_tree_widget;
//    QVBoxLayout* m_scroll_area_layout;
//    QVBoxLayout* m_container_layout;
//    QWidget* m_container;

//    //QLabel* m_item_deleted_label;

//    // NEW !!!!!;
//    QLabel* m_label;
//    QTableWidget* m_table_widget;

//    int m_tree_height;
//    int m_table_height;
//    int m_spacing = 1;

//    u32 m_current_id;

//    QTreeWidgetItem* m_source_item;
//    QTreeWidgetItem* m_destination_item;
};

#endif    // NET_DETAILS_WIDGET_H
