#include "gui/selection_details_widget/module_details_widget/module_ports_tree.h"

#include "gui/gui_globals.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/python/py_code_provider.h"
#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"
#include "gui/user_action/action_rename_object.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/utilities/enums.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QQueue>
#include <QDebug>

namespace hal
{
    ModulePortsTree::ModulePortsTree(QWidget* parent) : SizeAdjustableTreeView(parent), mPortModel(new ModulePinsTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);
        header()->setStretchLastSection(true);
        setModel(mPortModel);

        //connections
        //connect(this, &QTreeView::customContextMenuRequested, this, &ModulePortsTree::handleContextMenuRequested);
        connect(mPortModel, &ModulePinsTreeModel::numberOfPortsChanged, this, &ModulePortsTree::handleNumberOfPortsChanged);
    }

    void ModulePortsTree::setModule(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if (!m)
            return;

        mPortModel->setModule(m);
        mModuleID = moduleID;
        adjustSizeToContents();
    }

    void ModulePortsTree::setModule(Module* m)
    {
        if (!m)
            return;

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

    void ModulePortsTree::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex clickedIndex = indexAt(pos);
        if (!clickedIndex.isValid())
            return;

        TreeItem* clickedItem        = mPortModel->getItemFromIndex(clickedIndex);
        ModulePinsTreeModel::itemType type = mPortModel->getTypeOfItem(clickedItem);
        Net* n                       = mPortModel->getNetFromItem(clickedItem);
        QString name                 = clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString();
        u32 modId                    = mPortModel->getRepresentedModuleId();
        QMenu menu;

        //For now, if the item is a grouping item, only list of ports and namechange options
        if (type == ModulePinsTreeModel::itemType::portMultiBit)
        {
            menu.addAction(QIcon(":/icons/python"), "Extract ports as python list", [modId, name]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePortsOfGroup(modId, name)); });

            menu.move(mapToGlobal(pos));
            menu.exec();
            return;
        }

        //PLAINTEXT: NAME, DIRECTION, TYPE (for now, only port)
        menu.addAction("Extract name as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString()); });

        menu.addAction("Extract direction as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString()); });

        menu.addAction("Extract type as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sTypeColumn).toString()); });

        //Misc section: port renaming, selection change
        if (n)
        {
            bool isInputPort   = clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString().toStdString() == enum_to_string(PinDirection::input);
            QString renameText = isInputPort ? "Change input port name" : "Change output port name";

            menu.addSection("Misc");
            menu.addAction(renameText, [this, isInputPort, n, clickedItem]() {
                InputDialog ipd("Change port name", "New port name", clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString());
                if (ipd.exec() == QDialog::Accepted)
                {
                    ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                    act->setObject(UserActionObject(mModuleID, UserActionObjectType::Pin));
                    isInputPort ? act->setInputNetId(n->get_id()) : act->setOutputNetId(n->get_id());
                    act->exec();
                    setModule(mModuleID);
                }
            });

            menu.addAction("Add net to current selection", [this, n]() {
                gSelectionRelay->addNet(n->get_id());
                gSelectionRelay->relaySelectionChanged(this);
            });
        }

        //PYTHON: port, direction, type
        menu.addSection("Python");
        menu.addAction(QIcon(":/icons/python"), "Extract port as python code", [modId, name]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePortByName(modId, name)); });

        menu.addAction(QIcon(":/icons/python"), "Extract direction as python code", [modId, name]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePortDirection(modId, name)); });

        menu.addAction(QIcon(":/icons/python"), "Extract type as python code", [modId, name]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePortType(modId, name)); });

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void ModulePortsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        adjustSizeToContents();
        Q_EMIT updateText(QString("Pins (%1)").arg(newNumberPorts));
    }

}    // namespace hal
