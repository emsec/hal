#include "gui/new_selection_details_widget/debug_content_widget.h"

//------NetlistElements includes
#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "gui/new_selection_details_widget/models/base_tree_model.h"
#include "gui/new_selection_details_widget/models/pin_tree_model.h"
#include "gui/new_selection_details_widget/models/port_tree_model.h"
#include "gui/new_selection_details_widget/new_gate_details_widget/gate_pin_tree.h"
#include "gui/new_selection_details_widget/new_module_details_widget/module_elements_tree.h"
#include <QTreeView>
#include <QHeaderView>
//---------------------------

#include "gui/new_selection_details_widget/new_gate_details_widget/gate_info_table.h"
#include "gui/new_selection_details_widget/new_net_details_widget/net_info_table.h"
#include "gui/new_selection_details_widget/new_module_details_widget/module_info_table.h"
#include "gui/new_selection_details_widget/data_table_widget.h"
#include "gui/new_selection_details_widget/groupings_of_item_widget.h"
#include <QScrollArea>
#include <QGridLayout>
#include "gui/gui_globals.h"


//--------------------
#include "gui/new_selection_details_widget/new_net_details_widget/net_endpoint_table.h"
#include "gui/new_selection_details_widget/models/endpoint_table_model.h"

#include "gui/new_selection_details_widget/new_net_details_widget/net_module_table.h"
#include "gui/new_selection_details_widget/models/module_table_model.h"

#include "gui/new_selection_details_widget/details_frame_widget.h"

namespace hal {
    DebugContentWidget::DebugContentWidget(QWidget *parent) : ContentWidget("Debug Widget", parent)
    {
        QGridLayout *scrollAreaLayout = new QGridLayout(this);
        
        QWidget* scrollAreaContent = new QWidget(this);
        scrollAreaContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        scrollAreaContent->setLayout(scrollAreaLayout);
        scrollAreaLayout->setContentsMargins(0, 0, 0, 0);
        scrollAreaLayout->setSpacing(30);

        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        
        scrollArea->setWidgetResizable( true );
        scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        scrollArea->setWidget(scrollAreaContent);
        
        scrollAreaLayout->setSizeConstraint(QLayout::SetNoConstraint);
        mContentLayout->addWidget(scrollArea);
        

        // === Add your widgets here ===

        // --- Fabian Widgets ---
        DataTableWidget* dataTableWidget = new DataTableWidget(this);
        GroupingsOfItemWidget* groupingsOfItemWidget = new GroupingsOfItemWidget(this);
        LUTTableWidget* lutTableWidget = new LUTTableWidget(this);
        //BooleanFunctionTable* booleanFunctionTable = new BooleanFunctionTable(this);

        DetailsFrameWidget* dataTableWidgetFrame = new DetailsFrameWidget(dataTableWidget, "test", this);
        DetailsFrameWidget* groupingsOfItemWidgetFrame = new DetailsFrameWidget(groupingsOfItemWidget, "test", this);
        DetailsFrameWidget* lutTableWidgetFrame = new DetailsFrameWidget(lutTableWidget, "test", this);
        //DetailsFrameWidget* booleanFunctionTableFrame = new DetailsFrameWidget(booleanFunctionTable, "test", this);

        scrollAreaLayout->addWidget(dataTableWidgetFrame);
        scrollAreaLayout->addWidget(groupingsOfItemWidgetFrame);
        scrollAreaLayout->addWidget(lutTableWidgetFrame);
        //scrollAreaLayout->addWidget(booleanFunctionTableFrame);

        //connect(dataTableWidget,        &DataTableWidget::updateText,       dataTableWidgetFrame,       &DetailsFrameWidget::setText);
        //connect(groupingsOfItemWidget,  &GroupingsOfItemWidget::updateText, groupingsOfItemWidgetFrame, &DetailsFrameWidget::setText);
        //connect(lutTableWidget,         &LUTTableWidget::updateText,        lutTableWidgetFrame,        &DetailsFrameWidget::setText);


        //--------For NetlistElementsModel
        //NOTE: I've commented it out temporarily, because it SIGSEGVs sometimes (in netlists without gate id 2)
        QTreeView* mTreeView = new QTreeView(this);
        mTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mTreeView->header()->setStretchLastSection(true);
        NetlistElementsTreeModel* model = new NetlistElementsTreeModel(mTreeView);
        BaseTreeModel* baseModel = new BaseTreeModel(mTreeView);
        PinTreeModel* pinModel = new PinTreeModel(mTreeView);
        PortTreeModel* portModel = new PortTreeModel(mTreeView);
        baseModel->setHeaderLabels(QList<QVariant>() << "Label1" << "Label2");
        TreeItem* someRootItem = new TreeItem(QList<QVariant>() << "parent1" << "parent2");
        TreeItem* someChildItem = new TreeItem(QList<QVariant>() << "Test1" << "Test2");
        someRootItem->appendChild(someChildItem);
        baseModel->setContent(QList<TreeItem*>() << someRootItem);
        mTreeView->setModel(portModel);
        //mTreeView->setModel(model);
        scrollAreaLayout->addWidget(mTreeView);
        GatePinTree* gp = new GatePinTree(this);
        gp->setGate(19);
        scrollAreaLayout->addWidget(gp);
        ModuleElementsTree* met = new ModuleElementsTree(this);
        met->setModule(1);
        
        scrollAreaLayout->addWidget(met);

        
        //--------End NetlistElementsModel


        //General Info Tables
        GateInfoTable* gateTable = new GateInfoTable(this); //hardcoded to show gate with id 11 in constructor atm
        scrollAreaLayout->addWidget(gateTable);
        gateTable->setGate(gNetlist->get_gate_by_id(11));
        NetInfoTable* netTable = new NetInfoTable(this); //hardcoded to show net with id 17 in constructor atm
        scrollAreaLayout->addWidget(netTable);
        netTable->setNet(gNetlist->get_net_by_id(17));
        ModuleInfoTable* moduleTable = new ModuleInfoTable(this); //hardcoded to show module with id 1 in constructor atm
        scrollAreaLayout->addWidget(moduleTable);
        moduleTable->setModule(gNetlist->get_module_by_id(1));
        //-----------------------------------------------------------------------------------------------------------------------

        EndpointTableModel* sourceModel = new EndpointTableModel(EndpointTableModel::Type::source, this);
        NetEndpointTable* sourceTable = new NetEndpointTable(sourceModel, this);
        sourceTable->setContent(28);
        scrollAreaLayout->addWidget(sourceTable);

        EndpointTableModel* destinationModel = new EndpointTableModel(EndpointTableModel::Type::destination, this);
        NetEndpointTable* destinatinTable = new NetEndpointTable(destinationModel, this);
        destinatinTable->setContent(28);
        //scrollAreaLayout->addWidget(destinatinTable);

        ModuleTableModel* moduleModel = new ModuleTableModel(this);
        NetModuleTable* netModuleTable = new NetModuleTable(moduleModel, this);
        destinatinTable->setContent(28);
        scrollAreaLayout->addWidget(netModuleTable);

        DetailsFrameWidget* frame = new DetailsFrameWidget(destinatinTable, "test", this);
        scrollAreaLayout->addWidget(frame);

        connect(destinatinTable, &NetEndpointTable::update_text, frame, &DetailsFrameWidget::setText);
    }

} // namespace hal
