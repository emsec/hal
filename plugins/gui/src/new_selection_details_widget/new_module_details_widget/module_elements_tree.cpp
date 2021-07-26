#include "gui/new_selection_details_widget/new_module_details_widget/module_elements_tree.h"
#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "gui/new_selection_details_widget/py_code_provider.h"
#include "gui/gui_globals.h"
#include <QMenu>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QClipboard>

namespace hal
{

    ModuleElementsTree::ModuleElementsTree(QWidget *parent) : QTreeView(parent), mNetlistElementsModel(new NetlistElementsTreeModel(this)),
        mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        header()->setStretchLastSection(true);
        setModel(mNetlistElementsModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModuleElementsTree::handleContextMenuRequested);
    }

    void ModuleElementsTree::setContent(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        mNetlistElementsModel->setModule(m);
        mModuleID = moduleID;
    }

    void ModuleElementsTree::setContent(Module *m)
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
        TreeItem* clickedItem = mNetlistElementsModel->getItemFromIndex(indexAt(pos));
        QMenu menu;
        //Strings for first menu entry (python get net/gate/module)
        QString getObjectDescription, pythonGetObject;
        switch (mNetlistElementsModel->getTypeOfItem(clickedItem))
        {
            case NetlistElementsTreeModel::itemType::module:
                getObjectDescription = "Extract module as python code";
                pythonGetObject = PyCodeProvider::pyCodeModule(mNetlistElementsModel->getRepresentedIdOfItem(clickedItem)); break;
            case NetlistElementsTreeModel::itemType::gate:
                getObjectDescription = "Extract gate as python code";
                pythonGetObject = PyCodeProvider::pyCodeGate(mNetlistElementsModel->getRepresentedIdOfItem(clickedItem)); break;
            case NetlistElementsTreeModel::itemType::net:
                getObjectDescription = "Extract net as python code";
                pythonGetObject = PyCodeProvider::pyCodeNet(mNetlistElementsModel->getRepresentedIdOfItem(clickedItem)); break;
        }

        menu.addAction(QIcon(":/icons/python"), getObjectDescription,
           [pythonGetObject]()
           {
               QApplication::clipboard()->setText(pythonGetObject);
           }
        );


        menu.move(this->mapToGlobal(pos));
        menu.exec();
    }

}
