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
    /*forward declaration*/
    class GraphNavigationWidget;
    class DataFieldsTable;
    class DetailsSectionWidget;
    class DetailsGeneralModel;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Displays specific module details.
     *
     * The ModuleDetailsWidget class displays detailed information about a module including name, type
     * and input as well as output ports by calling its update(module-id) function. It is embedded within the
     * SelectionDetailsWidget and uses multiple tables to display the module's information.
     */
    class ModuleDetailsWidget : public DetailsWidget
    {
        Q_OBJECT
    public:

        /**
         * The constructor. Everything is set up here.
         *
         * @param parent - The widget's parent.
         */
        ModuleDetailsWidget(QWidget* parent = nullptr);

        /**
          * The destructor. The widget's navigation table is deleted here.
          */
        ~ModuleDetailsWidget();

        /**
         * Updates the information within the tables and section headers for the given module.
         *
         * @param module_id - The module id for which the information should be displayed.
         */
        void update(const u32 module_id);

        /**
         * Hides or shows all empty sections.
         *
         * @param hide - True to hide empty sections, False to show.
         */
        void hideSectionsWhenEmpty(bool hide) override;

    public Q_SLOTS:

        /**
         * Checks if the given net - that was marked as a global input net - is displayed
         * by the currently shown module. If yes, the displayed data is updated. Should be connected
         * to a netlist-marked-global-input signal.
         *
         * @param netlist - The netlist the net belongs to.
         * @param associated_data - The net that was marked as a global input.
         */
        void handleNetlistMarkedGlobalInput(Netlist* netlist, u32 associated_data);

        /**
         * Checks if the given net - that was marked as a global output net - is displayed
         * by the currently shown module. If yes, the displayed data is updated. Should be connected
         * to a netlist-marked-global-output signal.
         *
         * @param netlist - The netlist the net belongs to.
         * @param associated_data - The net that was marked as a global output.
         */
        void handleNetlistMarkedGlobalOutput(Netlist* netlist, u32 associated_data);

        /**
         * Checks if the given net - that was marked as a global inout net - is displayed
         * by the currently shown module. If yes, the displayed data is updated. Should be connected
         * to a netlist-marked-global-inout signal.
         *
         * @param netlist - The netlist the net belongs to.
         * @param associated_data - The net that was marked as a global inout.
         */
        void handleNetlistMarkedGlobalInout(Netlist* netlist, u32 associated_data);

        /**
         * Checks if the given net - that was unmarked from a global input net - is displayed
         * by the currently shown module. If yes, the displayed data is updated. Should be connected
         * to a netlist-unmarked-global-input signal.
         *
         * @param netlist - The netlist the net belongs to.
         * @param associated_data - The net that was unmarked.
         */
        void handleNetlistUnmarkedGlobalInput(Netlist* netlist, u32 associated_data);

        /**
         * Checks if the given net - that was unmarked from a global ouput net - is displayed
         * by the currently shown module. If yes, the displayed data is updated. Should be connected
         * to a netlist-unmarked-global-output signal.
         *
         * @param netlist - The netlist the net belongs to.
         * @param associated_data - The net that was unmarked.
         */
        void handleNetlistUnmarkedGlobalOutput(Netlist* netlist, u32 associated_data);

        /**
         * Checks if the given net - that was unmarked from a global inout net - is displayed
         * by the currently shown module. If yes, the displayed data is updated. Should be connected
         * to a netlist-unmarked-global-inout signal.
         *
         * @param netlist - The netlist the net belongs to.
         * @param associated_data - The net that was unmarked.
         */
        void handleNetlistUnmarkedGlobalInout(Netlist* netlist, u32 associated_data);

        /**
         * Checks if the given module is the same as the currently displayed module. If yes, the
         * displayed data is updated. Should be connected to a module-name-change signal.
         *
         * @param module - The module which name was changed.
         */
        void handleModuleNameChanged(Module* module);

        /**
         * Checks if the given module is currently displayed or if it is contained in the currently
         * displayed module. If yes, the data is updated. Should be connected to a submodule-added
         * signal.
         *
         * @param module - The module to which a submodule was added.
         * @param associated_data - The added module.
         */
        void handleSubmoduleAdded(Module* module, u32 associated_data);

        /**
         * Checks if the given module is currently displayed or if it is contained in the currently
         * displayed module. If yes, the data is updated. Should be connected to a submodule-removed
         * signal.
         *
         * @param module - The module from which a module was removed.
         * @param associated_data - The removed module.
         */
        void handleSubmoduleRemoved(Module* module, u32 associated_data);

        /**
         * Checks if the given module is currently displayed or if it is contained in the currently
         * displayed module. If yes, the data is updated. SHould be connected to a module-gate-assigned
         * signal.
         *
         * @param module - The module to which a gate was assigned.
         * @param associated_data - The assigned gate.
         */
        void handleModuleGateAssigned(Module* module, u32 associated_data);

        /**
         * Checks if the given module is currently displayed or if it is contained in the currently
         * displayed module. If yes, the data is updated. Should be connected to a module-gate-removed
         * signal.
         *
         * @param module - The module from which a gate was removed.
         * @param associated_data - The removed gate.
         */
        void handleModuleGateRemoved(Module* module, u32 associated_data);

        /**
         * Checks if the given module is the currently displayed module. If yes, the data is updated.
         * Should be connected to a module-input-port-name-changed signal.
         *
         * @param module - The module that had a input-port-name-change.
         * @param associated_data - The respective net.
         */
        void handleModuleInputPortNameChanged(Module* module, u32 associated_data);

        /**
         * Checks if the given module is the currently displayed module. If yes, the data is updated.
         * Should be connected to a module-output-name-change signal.
         *
         * @param module - The module that had a output-port-name-change.
         * @param associated_data - The respective net.
         */
        void handleModuleOutputPortNameChanged(Module* module, u32 associated_data);

        /**
         * Checks if the given module is currently displayed. If yes, the data is updated.
         * Should be connected to a module-type-change signal.
         *
         * @param module - The module which type was changed.
         */
        void handleModuleTypeChanged(Module* module);

        /**
         * Checks if the given net is a output or input net of the currently displayed module.
         * If yes, the data is updated. Should be connected to net-name-change signal.
         *
         * @param net - The net which name was changed.
         */
        void handleNetNameChanged(Net* net);

        /**
         * Checks if the given gate-id belongs to a gate that is contained in the currently displayed
         * module. If yes, the data is updated. Should be connected to a net-source-added signal.
         *
         * @param net - The net to which a new source was added.
         * @param src_gate_id - The added source.
         */
        void handleNetSourceAdded(Net* net, const u32 src_gate_id);

        /**
         * Checks if the given gate-id belongs to a gate that is contained in the currently displayed
         * module. If yes, the data is updated. Should be connected to a net-source-removed signal.
         *
         * @param net - The net from which a source was removed.
         * @param src_gate_id - The removed source.
         */
        void handleNetSourceRemoved(Net* net, const u32 src_gate_id);

        /**
         * Checks if the given gate-id belongs to a gate that is contained in the currently displayed
         * module. If yes, the data is updated. Should be connected to a net-destination-added signal.
         *
         * @param net - The net to which a destination was added.
         * @param dst_gate_id - The added destination.
         */
        void handleNetDestinationAdded(Net* net, const u32 dst_gate_id);

        /**
         * Checks if the given gate-id belongs to a gate that is contained in the currently displayed
         * module. If yes, the data is updated. Should be connected to a net-destination-removed signal.
         *
         * @param net - The net from which a destination was removed.
         * @param dst_gate_id - The removed destination.
         */
        void handleNetDestinationRemoved(Net* net, const u32 dst_gate_id);

    private:
        GraphNavigationWidget* mNavigationTable;

        QScrollArea* mScrollArea;
        QWidget* mTopLvlContainer;
        QVBoxLayout* mTopLvlLayout;
        QVBoxLayout* mContentLayout;

        QPushButton* mGeneralInfoButton;
        DetailsSectionWidget* mInputPortsSection;
        DetailsSectionWidget* mOutputPortsSection;
        DetailsSectionWidget* mDataFieldsSection;

        QTableView* mGeneralView;
        DetailsGeneralModel* mGeneralModel;

        QTableWidget* mInputPortsTable;

        QTableWidget* mOutputPortsTable;

        DataFieldsTable* mDataFieldsTable;

        //most straightforward and basic custom-context implementation (maybe need to be more dynamic)
        void handleInputPortsTableMenuRequested(const QPoint& pos);
        void handleOutputPortsTableMenuRequested(const QPoint& pos);

        //jump logic
        void handleOutputNetItemClicked(const QTableWidgetItem* item);
        void handleInputNetItemClicked(const QTableWidgetItem* item);
        void handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates);
        void handleNavigationCloseRequested();
    };
}    // namespace hal
