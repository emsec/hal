#include "gui/new_selection_details_widget/debug_content_widget.h"

//------NetlistElements includes
#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include <QTreeView>
#include <QHeaderView>
//---------------------------

#include "gui/new_selection_details_widget/new_gate_details_widget/gate_info_table.h"
#include "gui/new_selection_details_widget/new_net_details_widget/net_info_table.h"
#include "gui/new_selection_details_widget/new_module_details_widget/module_info_table.h"

namespace hal {
    DebugContentWidget::DebugContentWidget(QWidget *parent) : ContentWidget("Debug Widget", parent)
    {
        //mContentLayout->addWidget(new BooleanFunctionTable(this));

        //--------For NetlistElementsModel
        //NOTE: I've commented it out temporarily, because it SIGSEGVs sometimes (in netlists without gate id 2)
        QTreeView* mTreeView = new QTreeView(this);
        mTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mTreeView->header()->setStretchLastSection(true);
        NetlistElementsTreeModel* model = new NetlistElementsTreeModel(mTreeView);
        mTreeView->setModel(model);
        mContentLayout->addWidget(mTreeView);

        //--------End NetlistElementsModel


        //General Info Tables - Why are the views so scuffed when embedded into the debug widget? pls elp! maybe style sheets?
        //GateInfoTable* gateTable = new GateInfoTable(this); //hardcoded to show gate with id 11 in constructor atm
        //mContentLayout->addWidget(gateTable);

        //NetInfoTable* netTable = new NetInfoTable(this); //hardcoded to show net with id 17 in constructor atm
        //mContentLayout->addWidget(netTable);

        //ModuleInfoTable* moduleTable = new ModuleInfoTable(this); //hardcoded to show module with id 1 in constructor atm
        //mContentLayout->addWidget(moduleTable);
        //-----------------------------------------------------------------------------------------------------------------------
    }

} // namespace hal
