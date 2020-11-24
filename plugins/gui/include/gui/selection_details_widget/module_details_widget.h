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
    /*forward declaration*/
    class GraphNavigationWidget;
    class DataFieldsTable;
    class DetailsSectionWidget;
    class DetailsGeneralModel;

    class ModuleDetailsWidget : public DetailsWidget
    {
        Q_OBJECT
    public:
        ModuleDetailsWidget(QWidget* parent = nullptr);
        ~ModuleDetailsWidget();

        void update(const u32 module_id);

    public Q_SLOTS:

        void handleNetlistMarkedGlobalInput(Netlist* netlist, u32 associated_data);
        void handleNetlistMarkedGlobalOutput(Netlist* netlist, u32 associated_data);
        void handleNetlistMarkedGlobalInout(Netlist* netlist, u32 associated_data);
        void handleNetlistUnmarkedGlobalInput(Netlist* netlist, u32 associated_data);
        void handleNetlistUnmarkedGlobalOutput(Netlist* netlist, u32 associated_data);
        void handleNetlistUnmarkedGlobalInout(Netlist* netlist, u32 associated_data);

        void handleModuleNameChanged(Module* module);
        void handleSubmoduleAdded(Module* module, u32 associated_data);
        void handleSubmoduleRemoved(Module* module, u32 associated_data);
        void handleModuleGateAssigned(Module* module, u32 associated_data);
        void handleModuleGateRemoved(Module* module, u32 associated_data);
        void handleModuleInputPortNameChanged(Module* module, u32 associated_data);
        void handleModuleOutputPortNameChanged(Module* module, u32 associated_data);
        void handleModuleTypeChanged(Module* module);

        void handleNetNameChanged(Net* net);
        void handleNetSourceAdded(Net* net, const u32 src_gate_id);
        void handleNetSourceRemoved(Net* net, const u32 src_gate_id);
        void handleNetDestinationAdded(Net* net, const u32 dst_gate_id);
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
