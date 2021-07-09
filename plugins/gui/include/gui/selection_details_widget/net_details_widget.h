//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Displays specific net details.
     *
     * The NetDetailsWidget class displays detailed information about a net including name, type
     * and destination / sources by calling its update(net-id) function. It is embedded within the
     * SelectionDetaisWidget and uses multiple tables to display the net's information.
     */
    class NetDetailsWidget : public DetailsWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor. Everything is set up here.
         *
         * @param parent - The widget's parent.
         */
        NetDetailsWidget(QWidget* parent = nullptr);

        /**
          * The destructor.
          */
        ~NetDetailsWidget();

        /**
         * Updates the information within the tables and section headers for the given net.
         *
         * @param mNetId - The net id for which the information should be displayed.
         */
        void update(u32 mNetId);

        /**
         * Hides or shows all empty sections.
         *
         * @param hide - True to hide empty sections, False to show.
         */
        void hideSectionsWhenEmpty(bool hide) override;

    public Q_SLOTS:

        /**
         * Not implemented yet.
         *
         * @param n - The removed net.
         */
        void handleNetRemoved(Net* n);

        /**
         * Updates the general information model. This function should be connected to
         * a relay's net-name-changed signal.
         *
         * @param n - The net which name was changed.
         */
        void handleNetNameChanged(Net* n);

        /**
         * Updates all information by calling update(). This function should be connected
         * to a relay's net-source-added signal.
         *
         * @param n - The net to which a source was added.
         * @param src_gate_id - The id of the added source (gate).
         */
        void handleNetSourceAdded(Net* n, const u32 src_gate_id);

        /**
         * Updates all information by calling update(). This function should be connected
         * to a relay's net-source-removed signal.
         *
         * @param n - The net from which a source was removed.
         * @param src_gate_id - The id of the removed source (gate).
         */
        void handleNetSourceRemoved(Net* n, const u32 src_gate_id);

        /**
         * Updates all information by calling update(). This function should be connected
         * to a relay's net-destination-added signal.
         *
         * @param n - The net to which a destination was added.
         * @param dst_gate_id - The id of the added destination (gate).
         */
        void handleNetDestinationAdded(Net* n, const u32 dst_gate_id);

        /**
         * Updates all information by calling update(). This function should be connected
         * to a relay's net-destination-removed signal.
         *
         * @param n - The net from which a destination was removed.
         * @param dst_gate_id - The id of the removed destination (gate).
         */
        void handleNetDestinationRemoved(Net* n, const u32 dst_gate_id);

        /**
         * Checks if the given gate is a source or destination of the currently displayed net.
         * If yes, the information is updated by calling update(). This function should be
         * connected to a relay's gate-name-changed signal.
         *
         * @param g - The gate that underwent a name change.
         */
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
