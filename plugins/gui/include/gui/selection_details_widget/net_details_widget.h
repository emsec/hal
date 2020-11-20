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

        void update(u32 mNetId);

    public Q_SLOTS:

        void handleNetRemoved(Net* n);
        void handleNetNameChanged(Net* n);
        void handleNetSourceAdded(Net* n, const u32 src_gate_id);
        void handleNetSourceRemoved(Net* n, const u32 src_gate_id);
        void handleNetDestinationAdded(Net* n, const u32 dst_gate_id);
        void handleNetDestinationRemoved(Net* n, const u32 dst_gate_id);
        void handleGateNameChanged(Gate* g);

    private:
        //utility objects to encapsulate the sections together to make it scrollable
        QScrollArea* mScrollArea;
        QWidget* mTopLvlContainer;
        QVBoxLayout* mTopLvlLayout;
        QVBoxLayout* mContentLayout;

        //buttons to fold/unfold the corresponding sections
        QPushButton* mGeneralInfoButton;
        DetailsSectionWidget* mSourcePinsSection;
        DetailsSectionWidget* mDestinationPinsSection;
        DetailsSectionWidget* mDataFieldsSection;

        //the sections to unfold
        //(1) general information section
        QTableView*       mGeneralView;
        DetailsGeneralModel* mGeneralModel;

        //(2) source_pins section
        QTableWidget* mSourcePinsTable;

        //(3) destination_pins section
        QTableWidget* mDestinationPinsTable;

        //(4) data-fields section
        DataFieldsTable* mDataFieldsTable;

        //function section
        void handleTableItemClicked(QTableWidgetItem* item);

        //straightforward context menu handlers
        void handleSourcesTableMenuRequeted(const QPoint& pos);
        void handleDestinationsTableMenuRequeted(const QPoint& pos);
    };
}    // namespace hal
