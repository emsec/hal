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

#include "hal_core/defines.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/selection_details_widget/details_widget.h"

#include <QWidget>

/* forward declaration */
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QTableView;
class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QFont;

namespace hal
{
    /* forward declaration */
    class Net;
    class Gate;
    class DataFieldsTable;
    class DetailsSectionWidget;
    class DetailsGeneralModel;

    class NetDetailsWidget : public DetailsWidget
    {
        Q_OBJECT

    public:
        NetDetailsWidget(QWidget* parent = 0);
        ~NetDetailsWidget();

        virtual bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
        void update(u32 net_id);

    public Q_SLOTS:

        void handle_net_removed(Net* n);
        void handle_net_name_changed(Net* n);
        void handle_net_source_added(Net* n, const u32 src_gate_id);
        void handle_net_source_removed(Net* n, const u32 src_gate_id);
        void handle_net_destination_added(Net* n, const u32 dst_gate_id);
        void handle_net_destination_removed(Net* n, const u32 dst_gate_id);
        void handle_gate_name_changed(Gate* g);

    private:
        //utility objects to encapsulate the sections together to make it scrollable
        QScrollArea* m_scroll_area;
        QWidget* m_top_lvl_container;
        QVBoxLayout* m_top_lvl_layout;
        QVBoxLayout* m_content_layout;

        //buttons to fold/unfold the corresponding sections
        QPushButton* m_general_info_button;
        DetailsSectionWidget* m_sourcePinsSection;
        DetailsSectionWidget* m_destinationPinsSection;
        DetailsSectionWidget* m_dataFieldsSection;

        //the sections to unfold
        //(1) general information section
        QTableView*       mGeneralView;
        DetailsGeneralModel* mGeneralModel;
        QTableWidgetItem* m_name_item;
        QTableWidgetItem* m_type_item;
        QTableWidgetItem* m_id_item;

        //(2) source_pins section
        QTableWidget* m_source_pins_table;

        //(3) destination_pins section
        QTableWidget* m_destination_pins_table;

        //(4) data-fields section
        DataFieldsTable* m_dataFieldsTable;

        //function section
        void handle_table_item_clicked(QTableWidgetItem* item);

        //straightforward context menu handlers
        void handle_general_table_menu_requeted(const QPoint& pos);
        void handle_sources_table_menu_requeted(const QPoint& pos);
        void handle_destinations_table_menu_requeted(const QPoint& pos);

        //utility function, used to calculate the actual width so the scrollbars and the accuracy of the click functionality is correct
        QSize calculateTableSize(QTableView* table, int nrows, int ncols);
        QSize calculate_table_size(QTableWidget* table);
    };
}    // namespace hal
