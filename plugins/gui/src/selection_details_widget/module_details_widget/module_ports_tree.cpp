#include "gui/selection_details_widget/module_details_widget/module_ports_tree.h"
#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/input_dialog/input_dialog.h"
#include <QHeaderView>
#include <QQueue>
#include <QMenu>
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{

    ModulePortsTree::ModulePortsTree(QWidget *parent) : SizeAdjustableTreeView(parent), mPortModel(new PortTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);
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

        setModule(m->get_id());
    }

    void ModulePortsTree::removeContent()
    {
        mPortModel->clear();
        mModuleID = -1;
    }

    int ModulePortsTree::getRepresentedModuleId()
    {
        return mModuleID;
    }

    void ModulePortsTree::handleContextMenuRequested(const QPoint &pos)
    {
        QModelIndex clickedIndex = indexAt(pos);
        if(!clickedIndex.isValid())
            return;

        TreeItem* clickedItem = mPortModel->getItemFromIndex(clickedIndex);
        Net* n = mPortModel->getNetFromItem(clickedItem);

        //hacky to check if its an input or output port, change this when port-groupings
        //and real extensions for module type/direction is implemented (similar to the gate pin model)
        bool isInputPort = clickedItem->getData(PortTreeModel::sDirectionColumn).toString() == "input";
        QString renameText = isInputPort ? "Change input port name" : "Change output port name";

        QMenu menu;
        if(n)
        {
            menu.addAction(renameText, [this, isInputPort, n, clickedItem](){
                InputDialog ipd("Change port name", "New port name", clickedItem->getData(PortTreeModel::sNameColumn).toString());
                if(ipd.exec() == QDialog::Accepted)
                {
                    ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                    act->setObject(UserActionObject(mModuleID,UserActionObjectType::Port));
                    isInputPort ? act->setInputNetId(n->get_id()) : act->setOutputNetId(n->get_id());
                    act->exec();
                    setModule(mModuleID);
                }
            });
        }

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void ModulePortsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        adjustSizeToContents();
        Q_EMIT updateText(QString("Ports (%1)").arg(newNumberPorts));
    }


}
