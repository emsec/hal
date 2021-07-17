#include "gui/new_selection_details_widget/new_module_details_widget/module_elements_tree.h"
#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "gui/gui_globals.h"

namespace hal
{

    ModuleElementsTree::ModuleElementsTree(QWidget *parent) : QTreeView(parent), mNetlistElementsModel(new NetlistElementsTreeModel(this)),
        mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModuleElementsTree::handleContextMenuRequested);
    }

    void ModuleElementsTree::update(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        mNetlistElementsModel->setModule(m);
        mModuleID = moduleID;
    }

    void ModuleElementsTree::update(Module *m)
    {
        if(!m) return;

        mNetlistElementsModel->setModule(m);
        mModuleID = m->get_id();
    }

    void ModuleElementsTree::removeContent()
    {
        mNetlistElementsModel->clear();
        mModuleID = -1;
    }

    void ModuleElementsTree::handleContextMenuRequested(const QPoint &pos)
    {
        Q_UNUSED(pos)
    }

}
