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

#include "gui/gui_def.h"
#include "hal_core/netlist/endpoint.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/selection_details_widget/details_widget.h"

#include <QWidget>

class QTableWidget;
class QTableView;
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
    class DetailsGeneralModel;

    class GateDetailsWidget : public DetailsWidget
    {
        Q_OBJECT
    public:
        /** constructor */
        GateDetailsWidget(QWidget* parent = nullptr);
        /** destructor */
        ~GateDetailsWidget();

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

        void handleGateNameChanged(Gate* gate);
        void handleGateRemoved(Gate* gate);

        void handleModuleNameChanged(Module* module);
        void handleModuleRemoved(Module* module);
        void handleModuleGateAssigned(Module* module, u32 associated_data);
        void handleModuleGateRemoved(Module* module, u32 associated_data);

        void handleNetNameChanged(Net* net);
        void handleNetSourceAdded(Net* net, const u32 src_gate_id);
        void handleNetSourceRemoved(Net* net, const u32 src_gate_id);
        void handleNetDestinationAdded(Net* net, const u32 dst_gate_id);
        void handleNetDestinationRemoved(Net* net, const u32 dst_gate_id);

    private:
        //general
        //used to set the boolean function container to its appropriate size, width "must be"
        //extracted from the stylesheet
        int mScrollbarWidth;
        GraphNavigationWidget* mNavigationTable;

        //All sections together are encapsulated in a container to make it scrollable
        QScrollArea* mScrollArea;
        QWidget* mTopLvlContainer;
        QVBoxLayout* mTopLvlLayout;
        QVBoxLayout* mContentLayout;

        // buttons to fold/unfold corresponding sections
        QPushButton* mGeneralInfoButton;//(1)
        DetailsSectionWidget* mInputPinsSection;//(2)
        DetailsSectionWidget* mOutputPinsSection;//(3)
        DetailsSectionWidget* mDataFieldsSection;//(4)
        DetailsSectionWidget* mBooleanFunctionsSection;//(5)

        // widgets / sections to be unfold (not all structures are sections in itself, it may be a container necessary)

        //(1) general-information section ("static" information)
        QTableView*       mGeneralView;
        DetailsGeneralModel* mGeneralModel;

        //(2) input-pins section
        QTableWidget* mInputPinsTable;

        //(3) output-pins section
        QTableWidget* mOutputPinsTable;

        //(4) data-fields section
        DataFieldsTable* mDataFieldsTable;

        //(5) boolean-function section (consisting of a container that encapsulates multiple labels and design structures)
        QWidget* mBooleanFunctionsContainer;
        QVBoxLayout* mBooleanFunctionsContainerLayout;

        //function section
        void handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates);

        void handleInputPinItemClicked(const QTableWidgetItem* item);
        void handleOutputPinItemClicked(const QTableWidgetItem* item);

        //most straightforward and basic custom-context implementation (maybe need to be more dynamic)
        void handleInputPinTableMenuRequested(const QPoint &pos);
        void handleOutputPinTableMenuRequested(const QPoint &pos);
        void handleDataTableMenuRequested(const QPoint &pos);
    };
}
