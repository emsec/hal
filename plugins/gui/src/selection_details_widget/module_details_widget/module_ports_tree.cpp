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
        //setSelectionMode(QAbstractItemView::NoSelection);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
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
        QMenu menu;

        //PLAINTEXT: NAME, DIRECTION, TYPE (shared with pins and groups
        menu.addAction("Extract name as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString()); });
        menu.addAction("Extract direction as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString()); });
        menu.addAction("Extract type as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sTypeColumn).toString()); });

        if(type == ModulePinsTreeModel::itemType::portMultiBit)//group specific context
        {
            menu.addSection("Misc");
            menu.addAction("Change group name", [name, modId](){
                InputDialog ipd("Change group name", "New group name", name);
                if(ipd.exec() == QDialog::Accepted)
                {
                    if(ipd.textValue().isEmpty())
                        return;
                    ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                    act->setObject(UserActionObject(modId, UserActionObjectType::PinGroup));
                    act->setPinOrPingroupIdentifier(name);
                    act->exec();
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
            menu.addAction("Rename pin", [modId, name](){
                InputDialog ipd("Change pin name", "New pin name", name);
                if(ipd.exec() == QDialog::Accepted)
                {
                    if(ipd.textValue().isEmpty())
                        return;
                    ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                    act->setObject(UserActionObject(modId, UserActionObjectType::Pin));
                    act->setPinOrPingroupIdentifier(name);
                    act->exec();
                }
            });
            menu.addAction("Add net to current selection", [this, n](){
                gSelectionRelay->addNet(n->get_id());
                gSelectionRelay->relaySelectionChanged(this);
            });

        }
        //multi-selection (part of misc)
        if(selectionModel()->selectedRows().size() > 1)
        {
            QList<TreeItem*> selectedItems;
            for(auto index : selectionModel()->selectedRows())
            {
                TreeItem* item = mPortModel->getItemFromIndex(index);
                if(mPortModel->getTypeOfItem(item) != ModulePinsTreeModel::itemType::portMultiBit)
                    selectedItems.append(item);
            }
            if(selectedItems.size() > 1)
            {
                menu.addAction("Add objects to new pin group", [selectedItems, modId](){
                   InputDialog ipd("Pingroup name", "New pingroup name", "ExampleName");
                   if(ipd.exec() == QDialog::Accepted && !ipd.textValue().isEmpty())
                   {
                       std::vector<ModulePin*> pins;//must be fetched before creating new group
                       auto mod = gNetlist->get_module_by_id(modId);
                       for(auto item : selectedItems)
                           pins.push_back(mod->get_pin(item->getData(ModulePinsTreeModel::sNameColumn).toString().toStdString()));
                       mod->create_pin_group(ipd.textValue().toStdString(), pins);
                   }
                });
            }
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
