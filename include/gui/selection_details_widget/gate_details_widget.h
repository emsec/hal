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

#ifndef __HAL_GATE_DETAILS_WIDGET_H__
#define __HAL_GATE_DETAILS_WIDGET_H__

#include "def.h"

#include "netlist_relay/netlist_relay.h"
#include "netlist/endpoint.h"

#include <QWidget>

#include "../plugins/quine_mccluskey/include/plugin_quine_mccluskey.h"

/* forward declaration */
class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class QScrollArea;
class QModelIndex;

class gate_details_widget : public QWidget
{
    Q_OBJECT
public:
    /** constructor */
    gate_details_widget(QWidget* parent = nullptr);
    /** destructor */
    ~gate_details_widget();

    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /**
     * Handles update of gate details widget to display information of selected gate.
     * update() is called by selection details widget
     *
     * @param[in] gate_id - Selected gate id
     * @returns void
     */
    void update(const u32 gate_id);

public Q_SLOTS:

    void handle_tree_size_change(QTreeWidgetItem* item);
    void handle_item_expanded(QTreeWidgetItem* item);
    void handle_item_collapsed(QTreeWidgetItem* item);
    void on_treewidget_item_clicked(QTreeWidgetItem* item, int column);
    void on_gate_selected(endpoint selected);
    void on_general_table_item_double_clicked(const QModelIndex &index);

    void handle_gate_name_changed(std::shared_ptr<gate> gate);
    void handle_gate_removed(std::shared_ptr<gate> gate);
    void handle_module_name_changed(std::shared_ptr<module> module);
    void handle_module_removed(std::shared_ptr<module> module);
    void handle_module_gate_assigned(std::shared_ptr<module> module, u32 associated_data);
    void handle_module_gate_removed(std::shared_ptr<module> module, u32 associated_data);

private:
    QVBoxLayout* m_content_layout;

    QTableWidget* m_general_table;
    QTableWidgetItem* m_name_item;
    QTableWidgetItem* m_type_item;
    QTableWidgetItem* m_id_item;
    QTableWidgetItem* m_module_item;

    // stores Boolean function
    QLabel* m_boolean_function;
    //stores the initial lut value
    QLabel* m_init_value;
    // stores input pin tree view
    QTreeWidgetItem* m_input_pins;
    // stores output pin tree view
    QTreeWidgetItem* m_output_pins;

    QLabel* m_item_deleted_label;

    // stores utility objects for input/output pin tree view
    QScrollArea* m_scroll_area;
    QTreeWidget* m_tree_widget;
    QVBoxLayout* m_scroll_area_layout;
    QVBoxLayout* m_container_layout;
    QWidget* m_container;

    u32 m_current_id;

    u64 m_last_click_time;

    // store pointer to Quine-McCluskey plugin (to display minimized Boolean function)
    std::shared_ptr<plugin_quine_mccluskey> m_qmc;
};

#endif /* __HAL_GATE_DETAILS_WIDGET_H__ */
