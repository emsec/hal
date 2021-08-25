#include "gui/new_selection_details_widget/new_module_details_widget/module_ports_tree.h"
#include "gui/new_selection_details_widget/models/port_tree_model.h"
#include "gui/gui_globals.h"

namespace hal
{

    ModulePortsTree::ModulePortsTree(QWidget *parent) : QTreeView(parent), mPortModel(new PortTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModulePortsTree::handleContextMenuRequested);
        connect(mPortModel, &PortTreeModel::numberOfPortsChanged, this, &ModulePortsTree::handleNumberOfPortsChanged);

    }

    void ModulePortsTree::update(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        mPortModel->setModule(m);
        mModuleID = moduleID;
    }

    void ModulePortsTree::update(Module *m)
    {
        if(!m) return;

        mPortModel->setModule(m);
        mModuleID = m->get_id();

    }

    void ModulePortsTree::removeContent()
    {
        mPortModel->clear();
        mModuleID = -1;
    }

    void ModulePortsTree::handleContextMenuRequested(const QPoint &pos)
    {
        Q_UNUSED(pos)
    }

    void ModulePortsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        Q_EMIT updateText(QString("Ports(%1)").arg(newNumberPorts));
    }


}
