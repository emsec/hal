#include "gui/new_selection_details_widget/debug_content_widget.h"

//------NetlistElements includes
#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include <QTreeView>
#include <QHeaderView>
//---------------------------

namespace hal {
    DebugContentWidget::DebugContentWidget(QWidget *parent) : ContentWidget("Debug Widget", parent)
    {
        mContentLayout->addWidget(new BooleanFunctionTable(this));

        //--------For NetlistElementsModel
        /* NOTE: I've commented it out temporarily, because it SIGSEGVs sometimes (in netlists without gate id 2)
        QTreeView* mTreeView = new QTreeView(this);
        mTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mTreeView->header()->setStretchLastSection(true);
        NetlistElementsTreeModel* model = new NetlistElementsTreeModel(mTreeView);
        mTreeView->setModel(model);
        mContentLayout->addWidget(mTreeView);
         */
        //--------End NetlistElementsModel
    }

} // namespace hal
