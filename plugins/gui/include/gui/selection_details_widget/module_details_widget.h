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

#include "hal_core/def.h"
#include "gui/gui_def.h"
#include "hal_core/netlist/endpoint.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/selection_details_widget/details_widget.h"

#include <QWidget>

class QTableWidget;
class QTableWidgetItem;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QFont;
class QPushButton;

namespace hal
{
    /*forward declaration*/
    class GraphNavigationWidget;
    class DataFieldsTable;
    class DetailsSectionWidget;

    class ModuleDetailsWidget : public DetailsWidget
    {
        Q_OBJECT
    public:
        ModuleDetailsWidget(QWidget* parent = nullptr);
        ~ModuleDetailsWidget();

        virtual bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;

        void update(const u32 module_id);

    public Q_SLOTS:

        void handle_netlist_marked_global_input(Netlist* netlist, u32 associated_data);
        void handle_netlist_marked_global_output(Netlist* netlist, u32 associated_data);
        void handle_netlist_marked_global_inout(Netlist* netlist, u32 associated_data);
        void handle_netlist_unmarked_global_input(Netlist* netlist, u32 associated_data);
        void handle_netlist_unmarked_global_output(Netlist* netlist, u32 associated_data);
        void handle_netlist_unmarked_global_inout(Netlist* netlist, u32 associated_data);

        void handle_module_name_changed(Module* module);
        void handle_submodule_added(Module* module, u32 associated_data);
        void handle_submodule_removed(Module* module, u32 associated_data);
        void handle_module_gate_assigned(Module* module, u32 associated_data);
        void handle_module_gate_removed(Module* module, u32 associated_data);
        void handle_module_input_port_name_changed(Module* module, u32 associated_data);
        void handle_module_output_port_name_changed(Module* module, u32 associated_data);
        void handle_module_type_changed(Module* module);

        void handle_net_name_changed(Net* net);
        void handle_net_source_added(Net* net, const u32 src_gate_id);
        void handle_net_source_removed(Net* net, const u32 src_gate_id);
        void handle_net_destination_added(Net* net, const u32 dst_gate_id);
        void handle_net_destination_removed(Net* net, const u32 dst_gate_id);

    private:
        GraphNavigationWidget* m_navigation_table;

        QScrollArea* m_scroll_area;
        QWidget* m_top_lvl_container;
        QVBoxLayout* m_top_lvl_layout;
        QVBoxLayout* m_content_layout;

        QPushButton* m_general_info_button;
        DetailsSectionWidget* m_inputPortsSection;
        DetailsSectionWidget* m_outputPortsSection;
        DetailsSectionWidget* m_dataFieldsSection;

        QTableWidget* m_general_table;

        QTableWidgetItem* m_name_item;
        QTableWidgetItem* m_id_item;
        QTableWidgetItem* m_type_item;
        QTableWidgetItem* m_number_of_gates_item;
        QTableWidgetItem* m_number_of_submodules_item;
        QTableWidgetItem* m_number_of_nets_item;

        QTableWidget* m_input_ports_table;

        QTableWidget* m_output_ports_table;

        DataFieldsTable* m_dataFieldsTable;

        QSize calculate_table_size(QTableWidget* table);

        void add_general_table_static_item(QTableWidgetItem* item);
        void add_general_table_dynamic_item(QTableWidgetItem* item);

        //most straightforward and basic custom-context implementation (maybe need to be more dynamic)
        void handle_general_table_menu_requested(const QPoint& pos);
        void handle_input_ports_table_menu_requested(const QPoint& pos);
        void handle_output_ports_table_menu_requested(const QPoint& pos);

        //jump logic
        void handle_output_net_item_clicked(const QTableWidgetItem* item);
        void handle_input_net_item_clicked(const QTableWidgetItem* item);
        void handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates);

    };
}    // namespace hal
