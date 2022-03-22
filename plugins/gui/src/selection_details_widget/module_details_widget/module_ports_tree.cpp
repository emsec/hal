#include "gui/selection_details_widget/module_details_widget/module_ports_tree.h"

#include "gui/gui_globals.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/input_dialog/pingroup_selector_dialog.h"
#include "gui/python/py_code_provider.h"
#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_create_object.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/utilities/enums.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QQueue>
#include <QList>
#include <QSet>
#include <QMessageBox>

namespace hal
{
    ModulePinsTree::ModulePinsTree(QWidget* parent) : SizeAdjustableTreeView(parent), mPortModel(new ModulePinsTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //setSelectionMode(QAbstractItemView::NoSelection);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        //setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setFocusPolicy(Qt::NoFocus);
        header()->setStretchLastSection(true);
        setModel(mPortModel);

        setDragDropMode(DragDropMode::DragDrop);
        setDragEnabled(true);
        setAcceptDrops(true);
        viewport()->setAcceptDrops(true);
        setDropIndicatorShown(true);

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
        auto mod = gNetlist->get_module_by_id(modId);
        QList<TreeItem*> selectedPins;
        std::pair<bool, int> sameGroup; bool onlyPins;
        std::tie(selectedPins, sameGroup, onlyPins) = getSelectedPins();
        int itemId                   = mPortModel->getIdOfItem(clickedItem);
        QMenu menu;

        //shared plaintext entries: NAME, DIRECTION, TYPE (shared with pins and groups
        menu.addAction("Extract name as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString()); });
        menu.addAction("Extract direction as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString()); });
        menu.addAction("Extract type as plain text", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sTypeColumn).toString()); });

        menu.addSection("Misc");

        //shared context menu entry to add to existing groups
        bool addToExistingActionPossible = false;
        for(auto pingroup : mod->get_pin_groups())
        {
            if(pingroup->size() > 1)//at least one pingroup should have at least 2 items
            {
                addToExistingActionPossible = true;
                break;
            }
        }
        if(addToExistingActionPossible)
        {
            menu.addAction("Add selection to existing pin group", [this, selectedPins, mod](){
               PingroupSelectorDialog psd("Pingroup selector", "Select pingroup", mod);
               if(psd.exec() == QDialog::Accepted)
               {
                   std::vector<ModulePin*> pins;//must be fetched before creating new group
                   auto pinGroupRes = mod->get_pin_group_by_id(psd.getSelectedGroupId());
                   if(pinGroupRes.is_error()) return;
                   for(auto item : selectedPins)
                   {
                       auto pinRes = mod->get_pin_by_id(mPortModel->getIdOfItem(item));
                       if(pinRes.is_error()) return;
                       pins.push_back(pinRes.get());
                   }
                   for(auto pin : pins)
                       mod->assign_pin_to_group(pinGroupRes.get(), pin);
               }
            });
        }

        if(type == ModulePinsTreeModel::itemType::portMultiBit)//group specific context
        {
            menu.addAction("Rename pin group", [name, modId, itemId](){
                InputDialog ipd("Change pin group name", "New group name", name);
                if(ipd.exec() == QDialog::Accepted)
                {
                    if(ipd.textValue().isEmpty())
                        return;
                    auto groupResult = gNetlist->get_module_by_id(modId)->get_pin_group_by_id(itemId);
                    if (groupResult.is_ok())
                    {
                        ActionRenameObject* renameObj = new ActionRenameObject(ipd.textValue());
                        renameObj->setObject(UserActionObject(groupResult.get()->get_id(), UserActionObjectType::PinGroup));
                        renameObj->setParentObject(UserActionObject(modId, UserActionObjectType::Module));
                        renameObj->exec();
                    }
                }
            });
            menu.addAction("Delete pin group", [this, itemId, modId](){
                //add "are you sure?" dialog
                QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                          "Group deletion", "Are you sure you want to delete that group?", QMessageBox::Yes | QMessageBox::No);
                if(reply == QMessageBox::No)
                    return;
                auto mod = gNetlist->get_module_by_id(modId);
                if(!mod) return;
                auto pinGroupRes = mod->get_pin_group_by_id(itemId);
                if(pinGroupRes.is_ok()){
                    ActionDeleteObject* delObj = new ActionDeleteObject;
                    delObj->setObject(UserActionObject(pinGroupRes.get()->get_id(), UserActionObjectType::PinGroup));
                    delObj->setParentObject(UserActionObject(modId, UserActionObjectType::Module));
                    delObj->exec();
                }
            });

            if(selectionModel()->selectedRows().size() > 1)
                appendMultiSelectionEntries(menu, modId);

            menu.addSection("Python");
            menu.addAction(QIcon(":/icons/python"), "Extract pin group", [name, modId](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinGroup(modId, name));});
            menu.addAction(QIcon(":/icons/python"), "Extract pin group name", [name, modId](){QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinGroupName(modId, name));});
            menu.move(mapToGlobal(pos));
            menu.exec();
            return;
        }

        //menu.addSection("Misc");
        if(n)//should never be nullptr, but you never know
        {
            menu.addAction("Rename pin", [modId, name, itemId](){
                InputDialog ipd("Change pin name", "New pin name", name);
                if(ipd.exec() == QDialog::Accepted)
                {
                    if(ipd.textValue().isEmpty())
                        return;
                    auto pinResult = gNetlist->get_module_by_id(modId)->get_pin_by_id(itemId);
                    if(pinResult.is_ok())
                    {
                        ActionRenameObject* renameObj = new ActionRenameObject(ipd.textValue());
                        renameObj->setObject(UserActionObject(pinResult.get()->get_id(), UserActionObjectType::Pin));
                        renameObj->setParentObject(UserActionObject(modId, UserActionObjectType::Module));
                        renameObj->exec();
                    }
                }
            });
            menu.addAction("Add net to current selection", [this, n](){
                gSelectionRelay->addNet(n->get_id());
                gSelectionRelay->relaySelectionChanged(this);
            });

        }
        //can be both single(simple right-click, no real selection and multi-selection)
        //remove pin or pins from group
        if(sameGroup.first && mod->get_pin_group_by_id(sameGroup.second).get()->size() > 1)
        {
            menu.addAction("Remove selection from group", [this, selectedPins, mod](){
                std::vector<ModulePin*> pins;//must be fetched before creating new group
                for(auto item : selectedPins)
                {
                    auto pinRes = mod->get_pin_by_id(mPortModel->getIdOfItem(item));
                    if(pinRes.is_error()) return;
                    pins.push_back(mod->get_pin_by_id(mPortModel->getIdOfItem(item)).get());
                }
                for(auto pin : pins)
                    mod->remove_pin_from_group(pin->get_group().first, pin);
            });
        }

        //multi-selection (part of misc)
        if(selectionModel()->selectedRows().size() > 1)
            appendMultiSelectionEntries(menu, modId);

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

    void ModulePinsTree::appendMultiSelectionEntries(QMenu &menu, int modId)
    {
        QList<TreeItem*> selectedPins;
        std::pair<bool, int> sameGroup;
        bool onlyPins;
        std::tie(selectedPins, sameGroup, onlyPins) = getSelectedPins();
        if(selectedPins.size() > 1)
        {
            menu.addAction("Add objects to new pin group", [this, selectedPins, modId](){
               InputDialog ipd("Pingroup name", "New pingroup name", "ExampleName");
               if(ipd.exec() == QDialog::Accepted && !ipd.textValue().isEmpty())
               {
                   std::vector<ModulePin*> pins;//must be fetched before creating new group
                   auto mod = gNetlist->get_module_by_id(modId);
                   for(auto item : selectedPins)
                   {
                       auto pinRes = mod->get_pin_by_id(mPortModel->getIdOfItem(item));
                       if(pinRes.is_error())
                           return;
                       pins.push_back(pinRes.get());
                   }
                   mod->create_pin_group(ipd.textValue().toStdString(), pins);
               }
            });
        }
    }

    std::tuple<QList<TreeItem *>, std::pair<bool, int>, bool> ModulePinsTree::getSelectedPins()
    {
        QList<TreeItem*> selectedPins;//ordered
        QSet<TreeItem*> alreadyProcessedPins;//only for performance purposes
        bool sameGroup = true;
        bool onlyPins = true;
        int groupId = -1;
        for(auto index : selectionModel()->selectedRows())
        {
            TreeItem* item = mPortModel->getItemFromIndex(index);
            auto itemType = mPortModel->getTypeOfItem(item);
            if(itemType == ModulePinsTreeModel::itemType::pin)
            {
                if(!alreadyProcessedPins.contains(item))
                {
                    selectedPins.append(item);
                    alreadyProcessedPins.insert(item);
                }
            }
            else if(itemType == ModulePinsTreeModel::itemType::portMultiBit)
            {
                onlyPins = false;
                for(auto pinItem : item->getChildren())
                {
                    if(!alreadyProcessedPins.contains(pinItem))
                    {
                        selectedPins.append(pinItem);
                        alreadyProcessedPins.insert(pinItem);
                    }
                }
            }
        }
        //Check if all pins are from the same group (if no pin is selected it is from the same group)
        if(!selectedPins.isEmpty())
        {
            auto mod = gNetlist->get_module_by_id(mModuleID);
            auto firstPinRes = mod->get_pin_by_id(mPortModel->getIdOfItem(selectedPins.front()));
            auto firstPin = firstPinRes.get();
            groupId = firstPin->get_group().first->get_id();
            for(auto pinTreeItem : selectedPins)
            {
                auto pin = mod->get_pin_by_id(mPortModel->getIdOfItem(pinTreeItem)).get();
                if(groupId != (int)pin->get_group().first->get_id())
                {
                    sameGroup = false;
                    break;
                }
            }
            groupId = sameGroup ? groupId : -1;
        }
        return std::make_tuple(selectedPins, std::make_pair(sameGroup, groupId), onlyPins);
    }

}    // namespace hal
