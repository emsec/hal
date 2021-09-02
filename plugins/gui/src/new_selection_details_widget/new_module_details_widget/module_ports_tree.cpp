#include "gui/new_selection_details_widget/new_module_details_widget/module_ports_tree.h"
#include "gui/new_selection_details_widget/models/port_tree_model.h"
#include <QHeaderView>
#include <QQueue>
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{

    ModulePortsTree::ModulePortsTree(QWidget *parent) : SizeAdjustableTreeView(parent), mPortModel(new PortTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        header()->setStretchLastSection(true);
        setModel(mPortModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModulePortsTree::handleContextMenuRequested);
        connect(mPortModel, &PortTreeModel::numberOfPortsChanged, this, &ModulePortsTree::handleNumberOfPortsChanged);

    }

    void ModulePortsTree::setModule(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        mPortModel->setModule(m);
        mModuleID = moduleID;
        adjustSizeToContents();
    }

    void ModulePortsTree::setModule(Module *m)
    {
        if(!m) return;

        mPortModel->setModule(m);
        adjustSizeToContents();
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
        adjustSizeToContents();
        Q_EMIT updateText(QString("Ports (%1)").arg(newNumberPorts));
    }


}
