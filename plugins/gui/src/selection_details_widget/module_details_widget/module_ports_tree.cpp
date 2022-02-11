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
    ModulePinsTree::ModulePinsTree(QWidget* parent) : SizeAdjustableTreeView(parent), mPortModel(new ModulePinsTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);
        header()->setStretchLastSection(true);
        setModel(mPortModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModulePinsTree::handleContextMenuRequested);
        connect(mPortModel, &ModulePinsTreeModel::numberOfPortsChanged, this, &ModulePinsTree::handleNumberOfPortsChanged);
    }

    void ModulePinsTree::setModule(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if (!m)
            return;

        mPortModel->setModule(m);
        mModuleID = moduleID;
        adjustSizeToContents();
    }

    void ModulePinsTree::setModule(Module* m)
    {
        if (!m)
            return;

        setModule(m->get_id());
    }

    void ModulePinsTree::removeContent()
    {
        mPortModel->clear();
        mModuleID = -1;
    }

    int ModulePinsTree::getRepresentedModuleId()
    {
        return mModuleID;
    }

    void ModulePinsTree::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex clickedIndex = indexAt(pos);
        if (!clickedIndex.isValid())
            return;

        //all relevant information
        TreeItem* clickedItem        = mPortModel->getItemFromIndex(clickedIndex);
        ModulePinsTreeModel::itemType type = mPortModel->getTypeOfItem(clickedItem);
        Net* n                       = mPortModel->getNetFromItem(clickedItem);
        QString name                 = clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString();
        u32 modId                    = mPortModel->getRepresentedModuleId();
        int itemId                   = mPortModel->getIdOfItem(clickedItem);
        QMenu menu;

        //PLAINTEXT: NAME, DIRECTION, TYPE (shared with pins and groups
        menu.addAction("Extract name as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString()); });
        menu.addAction("Extract direction as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString()); });
        menu.addAction("Extract type as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sTypeColumn).toString()); });

        if(type == ModulePinsTreeModel::itemType::portMultiBit)//group specific context
        {
            menu.addSection("Misc");
            menu.addAction("Change group name", [name, modId, itemId](){
                InputDialog ipd("Change group name", "New group name", name);
                if(ipd.exec() == QDialog::Accepted)
                {
                    auto groupResult = gNetlist->get_module_by_id(modId)->get_pin_group_by_id(itemId);
                    if (groupResult.is_ok())
                        gNetlist->get_module_by_id(modId)->set_pin_group_name(groupResult.get(), ipd.textValue().toStdString());
                }
            });
            menu.addSection("Python");
            menu.addAction(QIcon(":/icons/python"), "Extract pin group", [name, modId](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinGroup(modId, name));});
            menu.addAction(QIcon(":/icons/python"), "Extract pin group name", [name, modId](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinGroupName(modId, name));});
            menu.move(mapToGlobal(pos));
            menu.exec();
            return;
        }

        menu.addSection("Misc");
        if(n)//should never be nullptr, but you never know
        {
            menu.addAction("Rename pin", [modId, name, itemId](){
                InputDialog ipd("Change pin name", "New pin name", name);
                if(ipd.exec() == QDialog::Accepted)
                {
                    auto pinResult = gNetlist->get_module_by_id(modId)->get_pin_by_id(itemId);
                    if(pinResult.is_ok())
                        gNetlist->get_module_by_id(modId)->set_pin_name(pinResult.get(), ipd.textValue().toStdString());
                }
            });
            menu.addAction("Add net to current selection", [this, n](){
                gSelectionRelay->addNet(n->get_id());
                gSelectionRelay->relaySelectionChanged(this);
            });

        }

        menu.addSection("Python");
        menu.addAction(QIcon(":/icons/python"), "Extract pin object", [modId, name](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinByName(modId, name));});
        menu.addAction(QIcon(":/icons/python"), "Extract pin name", [modId, name](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinName(modId, name));});
        menu.addAction(QIcon(":/icons/python"), "Extract pin direction", [modId, name](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinDirection(modId, name));});
        menu.addAction(QIcon(":/icons/python"), "Extract pin type", [modId, name](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinType(modId, name));});

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void ModulePinsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        adjustSizeToContents();
        Q_EMIT updateText(QString("Pins (%1)").arg(newNumberPorts));
    }

}    // namespace hal
