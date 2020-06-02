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

#include "gui/gui_def.h"
#include "netlist/endpoint.h"
#include "netlist_relay/netlist_relay.h"

#include <QWidget>

/* forward declaration */
class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QGridLayout;
class QModelIndex;
class QFont;
class graph_navigation_widget;

class QPushButton;

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
    void update2(const u32 gate_id);

public Q_SLOTS:

    void on_treewidget_item_clicked(QTreeWidgetItem* item, int column);
    void handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates);
    void on_general_table_item_double_clicked(const QModelIndex& index);

    void handle_gate_name_changed(std::shared_ptr<gate> gate);
    void handle_gate_removed(std::shared_ptr<gate> gate);

    void handle_module_name_changed(std::shared_ptr<module> module);
    void handle_module_removed(std::shared_ptr<module> module);
    void handle_module_gate_assigned(std::shared_ptr<module> module, u32 associated_data);
    void handle_module_gate_removed(std::shared_ptr<module> module, u32 associated_data);

    void handle_net_name_changed(std::shared_ptr<net> net);
    void handle_net_source_added(std::shared_ptr<net> net, const u32 src_gate_id);
    void handle_net_source_removed(std::shared_ptr<net> net, const u32 src_gate_id);
    void handle_net_destination_added(std::shared_ptr<net> net, const u32 dst_gate_id);
    void handle_net_destination_removed(std::shared_ptr<net> net, const u32 dst_gate_id);

private:
    //NEW CODE

    //general
    QFont m_key_font;

    //All sections together are encapsulated in a container to make it scrollable
    QScrollArea* m_scroll_area;
    QWidget* m_top_lvl_container;
    QVBoxLayout* m_top_lvl_layout;
    QVBoxLayout* m_content_layout;

    // buttons to fold/unfold corresponding sections
    QPushButton* m_general_info_button;//(1)
    QPushButton* m_input_pins_button;//(2)
    QPushButton* m_output_pins_button;//(3)
    QPushButton* m_data_fields_button;//(4)
    QPushButton* m_boolean_functions_button;//(5)

    // widgets / sections to be unfold (not all structures are sections in itself, it may be a container necessary)

    //(1) general-information section ("static" information)
    QTableWidget* m_general_table;
    QTableWidgetItem* m_name_item;
    QTableWidgetItem* m_type_item;
    QTableWidgetItem* m_id_item;
    QTableWidgetItem* m_module_item;

    //(2) input-pins section
    QTableWidget* m_input_pins_table;

    //(3) output-pins section
    QTableWidget* m_output_pins_table;

    //(4) data-fields section (label or also table? if table: maybe tooltip if data is too long)
    QTableWidget* m_data_fields_table;

    //(5) boolean-function section (consisting of a container that encapsulates multiple labels and design structures)
    QWidget* m_boolean_functions_container;
    QVBoxLayout* m_boolean_functions_container_layout;

    //function sections
    void handle_buttons_clicked();
    void handle_input_pin_item_clicked(QTableWidgetItem* item);
    void handle_output_pin_item_clicked(QTableWidgetItem* item);

    //most straightforward and basic custom-context implementation (maybe need to be more dynamic)
    void handle_general_table_menu_requested(const QPoint &pos);
    void handle_input_pin_table_menu_requested(const QPoint &pos);
    void handle_output_pin_table_menu_requested(const QPoint &pos);
    void handle_data_table_menu_requested(const QPoint &pos);
    void handle_python_action_clicked(bool checked);

    //utility function, used to calculate the actual width so the scrollbars and the accuracy of the click functionality is correct
    QSize calculate_table_size(QTableWidget* table);

    //OLD CODE
    u32 m_current_id;

    u64 m_last_click_time;

    graph_navigation_widget* m_navigation_table;

    //used to set the boolean function container to its appropriate size, width "must be"
    //extracted from the stylesheet
    int m_scrollbar_width;
};

#endif /* __HAL_GATE_DETAILS_WIDGET_H__ */
