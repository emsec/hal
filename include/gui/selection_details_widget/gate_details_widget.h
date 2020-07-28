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

#include "def.h"

#include "gui/gui_def.h"
#include "netlist/endpoint.h"
#include "netlist_relay/netlist_relay.h"
#include "selection_details_widget/details_widget.h"

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
    /* forward declaration */
    class GraphNavigationWidget;
    class DataFieldsTable;
    class DetailsSectionWidget;

    class GateDetailsWidget : public DetailsWidget
    {
        Q_OBJECT
    public:
        /** constructor */
        GateDetailsWidget(QWidget* parent = nullptr);
        /** destructor */
        ~GateDetailsWidget();

        virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
        virtual bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

        /**
         * Handles update of gate details widget to display information of selected gate.
         * update() is called by selection details widget
         *
         * @param[in] gate_id - Selected gate id
         * @returns void
         */
        void update(const u32 gate_id);

    public Q_SLOTS:

        void handle_gate_name_changed(std::shared_ptr<Gate> gate);
        void handle_gate_removed(std::shared_ptr<Gate> gate);

        void handle_module_name_changed(std::shared_ptr<Module> module);
        void handle_module_removed(std::shared_ptr<Module> module);
        void handle_module_gate_assigned(std::shared_ptr<Module> module, u32 associated_data);
        void handle_module_gate_removed(std::shared_ptr<Module> module, u32 associated_data);

        void handle_net_name_changed(std::shared_ptr<Net> net);
        void handle_net_source_added(std::shared_ptr<Net> net, const u32 src_gate_id);
        void handle_net_source_removed(std::shared_ptr<Net> net, const u32 src_gate_id);
        void handle_net_destination_added(std::shared_ptr<Net> net, const u32 dst_gate_id);
        void handle_net_destination_removed(std::shared_ptr<Net> net, const u32 dst_gate_id);

    private:
        //general
        //used to set the boolean function container to its appropriate size, width "must be"
        //extracted from the stylesheet
        int m_scrollbar_width;
        GraphNavigationWidget* m_navigation_table;

        //All sections together are encapsulated in a container to make it scrollable
        QScrollArea* m_scroll_area;
        QWidget* m_top_lvl_container;
        QVBoxLayout* m_top_lvl_layout;
        QVBoxLayout* m_content_layout;

        // buttons to fold/unfold corresponding sections
        QPushButton* m_general_info_button;//(1)
        DetailsSectionWidget* m_inputPinsSection;//(2)
        DetailsSectionWidget* m_outputPinsSection;//(3)
        DetailsSectionWidget* m_dataFieldsSection;//(4)
        DetailsSectionWidget* m_booleanFunctionsSection;//(5)

        // widgets / sections to be unfold (not all structures are sections in itself, it may be a container necessary)

        //(1) general-information section ("static" information)
        QTableWidget* m_general_table;
        QTableWidgetItem* m_name_item;
        QTableWidgetItem* m_type_item;
        QTableWidgetItem* m_id_item;
        QTableWidgetItem* m_ModuleItem;

        //(2) input-pins section
        QTableWidget* m_input_pins_table;

        //(3) output-pins section
        QTableWidget* m_output_pins_table;

        //(4) data-fields section
        DataFieldsTable* m_dataFieldsTable;

        //(5) boolean-function section (consisting of a container that encapsulates multiple labels and design structures)
        QWidget* m_boolean_functions_container;
        QVBoxLayout* m_boolean_functions_container_layout;

        //function section
        void handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates);

        void handle_input_pin_item_clicked(const QTableWidgetItem* item);
        void handle_output_pin_item_clicked(const QTableWidgetItem* item);
        void handle_general_table_item_clicked(const QTableWidgetItem* item);

        //most straightforward and basic custom-context implementation (maybe need to be more dynamic)
        void handle_general_table_menu_requested(const QPoint &pos);
        void handle_input_pin_table_menu_requested(const QPoint &pos);
        void handle_output_pin_table_menu_requested(const QPoint &pos);
        void handle_data_table_menu_requested(const QPoint &pos);

        //utility function, used to calculate the actual width so the scrollbars and the accuracy of the click functionality is correct
        QSize calculate_table_size(QTableWidget* table);
    };
}
