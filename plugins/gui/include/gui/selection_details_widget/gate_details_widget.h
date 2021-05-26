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

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Displays specific gate details.
     *
     * The GateDetailsWidget class displays detailed information about a gate including name, type
     * and input / output pins by calling its update(gate-id) function. It is embedded in the
     * SelectionDetailsWidget and uses multiple tables to display the gate's information.
     */
    class GateDetailsWidget : public DetailsWidget
    {
        Q_OBJECT
    public:
        /** constructor */
        GateDetailsWidget(QWidget* parent = nullptr);
        /** destructor */
        ~GateDetailsWidget();

        virtual void resizeEvent(QResizeEvent* event) override;

        /**
         * Handles update of gate details widget to display information of selected gate.
         * update() is called by selection details widget
         *
         * @param[in] gate_id - Selected gate id
         * @returns void
         */
        void update(const u32 gate_id);

        /**
         * Hides or shows all empty sections.
         *
         * @param hide - True to hide empty sections, False to show.
         */
        void hideSectionsWhenEmpty(bool hide) override;

    public Q_SLOTS:

        /**
         * Checks if the given gate is the one that is currently displayed. If yes, the information
         * is updated. This function should be connected to a gate-name-changed signal.
         *
         * @param gate - The gate which name was changed.
         */
        void handleGateNameChanged(Gate* gate);

        /**
         * Checks if the given gate is currently dispalyed. If yes, the main scroll are (where the content is located)
         * is hidden. This function should be connected to a gate-removed signal.
         *
         * @param gate - The gate that was removed.
         */
        void handleGateRemoved(Gate* gate);

        /**
         * Checks if the currently displayed gate is contained in the given module. If yes, the information
         * is updated. Should be connected to a module-name-changed signal.
         *
         * @param module - The module which name was changed.
         */
        void handleModuleNameChanged(Module* module);

        /**
         * Checks if the currently displayed gate is contained in the given module. If yes, the information
         * is updated. This function should be connected to a module-removed signal.
         *
         * @param module - The removoed module.
         */
        void handleModuleRemoved(Module* module);

        /**
         * Checks if the currently displayed gate is the one that was assigned to a module. If yes,
         * the information is updated. This function should be connected to a module-gate-assigned signal.
         *
         * @param module - The module a gate was assigned to.
         * @param associated_data - The id of the assigned gate.
         */
        void handleModuleGateAssigned(Module* module, u32 associated_data);

        /**
         * Checks if the currently displayed gate is the one that was removed from a module. If yes,
         * the information is updated. This function should be connected to a module-gate-removed signal.
         *
         * @param module - The module a gate was removed from.
         * @param associated_data - The id of the removed gate.
         */
        void handleModuleGateRemoved(Module* module, u32 associated_data);

        /**
         * Checks if the given net is either a source or a destination of the currently displayed gate.
         * If yes, the information is updated. This function should be connected to a net-name-changed signal.
         *
         * @param net - The net that had a name-change.
         */
        void handleNetNameChanged(Net* net);

        /**
         * Checks if the given gate-id belongs to the gate that is currently displayed. If yes, the information
         * is updated. This function should be connected to a net-source-added signal.
         *
         * @param net - The net to which a source was added.
         * @param src_gate_id - The gate-id of the added source.
         */
        void handleNetSourceAdded(Net* net, const u32 src_gate_id);

        /**
         * Checks if the given gate-id belongs to the gate that is currently displayed. If yes, the information
         * is updated. This function should be connected to a net-source-removed signal.
         *
         * @param net - The net from which a source was removed.
         * @param src_gate_id - The gate-id of the removed source.
         */
        void handleNetSourceRemoved(Net* net, const u32 src_gate_id);

        /**
         * Checks if the given gate-id belongs to the gate that is currently displayed. If yes, the information
         * is updated. This function should be connected to a net-destination-added signal.
         *
         * @param net - The net to which a destination was added.
         * @param dst_gate_id - The gate-id of the added destination.
         */
        void handleNetDestinationAdded(Net* net, const u32 dst_gate_id);

        /**
         * Checks if the given gate-id belongs to the gate that is currently displayed. If yes, the information
         * is updated. This function should be connected to a net-destination-removed signal.
         *
         * @param net - The net from which a destination was removed.
         * @param dst_gate_id - The gate-id of the removed destination.
         */
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
        void handleNavigationCloseRequested();

        void handleInputPinItemClicked(const QTableWidgetItem* item);
        void handleOutputPinItemClicked(const QTableWidgetItem* item);

        //most straightforward and basic custom-context implementation (maybe need to be more dynamic)
        void handleInputPinTableMenuRequested(const QPoint &pos);
        void handleOutputPinTableMenuRequested(const QPoint &pos);
        void handleDataTableMenuRequested(const QPoint &pos);
    };
}
