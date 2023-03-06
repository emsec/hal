#include "gui/selection_details_widget/module_details_widget/module_ports_tree.h"

#include "gui/gui_globals.h"
#include "gui/input_dialog/combobox_dialog.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/input_dialog/pingroup_selector_dialog.h"
#include "gui/python/py_code_provider.h"
#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_set_object_type.h"
#include "gui/user_action/user_action_compound.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/utilities/enums.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QQueue>
#include <QSet>

namespace hal
{
    ModulePinsTree::ModulePinsTree(QWidget* parent) : QTreeView(parent), mPortModel(new ModulePinsTreeModel(this)), mModuleID(-1)
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
        TreeItem* clickedItem              = mPortModel->getItemFromIndex(clickedIndex);
        ModulePinsTreeModel::itemType type = mPortModel->getTypeOfItem(clickedItem);
        Net* n                             = mPortModel->getNetFromItem(clickedItem);
        QString name                       = clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString();
        u32 modId                          = mPortModel->getRepresentedModuleId();
        auto mod                           = gNetlist->get_module_by_id(modId);
        QList<TreeItem*> selectedPins;
        std::pair<bool, int> sameGroup;
        bool onlyPins;
        std::tie(selectedPins, sameGroup, onlyPins) = getSelectedPins();
        int itemId                                  = mPortModel->getIdOfItem(clickedItem);
        QMenu menu;

        //shared plaintext entries: NAME, DIRECTION, TYPE (shared with pins and groups
        menu.addAction("Name to clipboard", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString()); });
        menu.addAction("Direction to clipboard", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString()); });
        menu.addAction("Type to clipboard", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sTypeColumn).toString()); });

        menu.addSection("Misc");

        //shared context menu entry to add to existing groups
        bool addToExistingActionPossible = false;
        for (auto pingroup : mod->get_pin_groups())
        {
            if (pingroup->size() > 1)    //at least one pingroup should have at least 2 items
            {
                addToExistingActionPossible = true;
                break;
            }
        }
        if (addToExistingActionPossible)
        {
            menu.addAction("Add selection to existing pin group", [this, selectedPins, mod]() {
                PingroupSelectorDialog psd("Pingroup selector", "Select pingroup", mod);
                if (psd.exec() == QDialog::Accepted)
                {
                    QSet<u32> pinSet;
                    auto* pinGroup = mod->get_pin_group_by_id(psd.getSelectedGroupId());
                    if (pinGroup == nullptr)
                        return;
                    for (auto item : selectedPins)
                    {
                        auto* pin = mod->get_pin_by_id(mPortModel->getIdOfItem(item));
                        if (pin == nullptr)
                            return;
                        pinSet.insert(pin->get_id());
                    }
                    ActionAddItemsToObject* act = new ActionAddItemsToObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), pinSet);
                    act->setObject(UserActionObject(pinGroup->get_id(), UserActionObjectType::PinGroup));
                    act->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                    act->exec();
                }
            });
        }

        if (type == ModulePinsTreeModel::itemType::portMultiBit)    //group specific context, own helper function?
        {
            menu.addAction("Change name", [name, modId, itemId]() {
                InputDialog ipd("Change pin group name", "New group name", name);
                if (ipd.exec() == QDialog::Accepted)
                {
                    if (ipd.textValue().isEmpty())
                        return;
                    auto* group = gNetlist->get_module_by_id(modId)->get_pin_group_by_id(itemId);
                    if (group != nullptr)
                    {
                        ActionRenameObject* renameObj = new ActionRenameObject(ipd.textValue());
                        renameObj->setObject(UserActionObject(group->get_id(), UserActionObjectType::PinGroup));
                        renameObj->setParentObject(UserActionObject(modId, UserActionObjectType::Module));
                        renameObj->exec();
                    }
                }
            });
            menu.addAction("Delete pin group", [itemId, mod]() {
                auto* pinGroup = mod->get_pin_group_by_id(itemId);
                if (pinGroup != nullptr)
                {
                    ActionDeleteObject* delObj = new ActionDeleteObject;
                    delObj->setObject(UserActionObject(pinGroup->get_id(), UserActionObjectType::PinGroup));
                    delObj->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                    delObj->exec();
                }
            });

            if (selectionModel()->selectedRows().size() > 1)
                appendMultiSelectionEntries(menu, modId);

            menu.addSection("Python");
            menu.addAction(QIcon(":/icons/python"), "Get pin group", [modId, itemId]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinGroup(modId, itemId)); });
            menu.move(mapToGlobal(pos));
            menu.exec();
            return;
        }

        //menu.addSection("Misc");
        if (n)    //should never be nullptr, but you never know
        {
            menu.addAction("Change name", [mod, name, itemId]() {
                InputDialog ipd("Change pin name", "New pin name", name);
                if (ipd.exec() == QDialog::Accepted)
                {
                    if (ipd.textValue().isEmpty())
                        return;
                    auto* pin = mod->get_pin_by_id(itemId);
                    if (pin != nullptr)
                    {
                        ActionRenameObject* renameObj = new ActionRenameObject(ipd.textValue());
                        renameObj->setObject(UserActionObject(pin->get_id(), UserActionObjectType::Pin));
                        renameObj->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                        renameObj->exec();
                    }
                }
            });

            menu.addAction("Change type", [mod, name, itemId]() {
                auto* pin = mod->get_pin_by_id(itemId);
                if (pin == nullptr)
                    return;

                QStringList types;
                for (auto const& [k, v] : EnumStrings<PinType>::data)
                    types << QString::fromStdString(v);

                ComboboxDialog cbd("Pin Types", "Select pin type", types);

                if (cbd.exec() == QDialog::Accepted)
                {
                    ActionSetObjectType* act = new ActionSetObjectType(cbd.textValue());
                    act->setObject(UserActionObject(pin->get_id(), UserActionObjectType::Pin));
                    act->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                    act->exec();
                }
            });
            menu.addAction("Add net to current selection", [this, n]() {
                gSelectionRelay->addNet(n->get_id());
                gSelectionRelay->relaySelectionChanged(this);
            });
            menu.addAction("Set net as current selection", [this, n]() {
                gSelectionRelay->clear();
                gSelectionRelay->addNet(n->get_id());
                gSelectionRelay->relaySelectionChanged(this);
            });
        }
        //can be both single(simple right-click, no real selection) and multi-selection
        if (sameGroup.first && mod->get_pin_group_by_id(sameGroup.second)->size() > 1)
        {
            menu.addAction("Remove selection from group", [this, selectedPins, mod, sameGroup]() {
                QSet<u32> pins;
                for (auto item : selectedPins)
                    pins.insert(mPortModel->getIdOfItem(item));

                ActionRemoveItemsFromObject* act = new ActionRemoveItemsFromObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), pins);
                act->setObject(UserActionObject(mod->get_pin_group_by_id(sameGroup.second)->get_id(), UserActionObjectType::PinGroup));
                act->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                act->exec();
            });
        }

        //multi-selection (part of misc)
        if (selectionModel()->selectedRows().size() > 1)
            appendMultiSelectionEntries(menu, modId);

        menu.addSection("Python");
        menu.addAction(QIcon(":/icons/python"), "Get pin", [modId, itemId]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinById(modId, itemId)); });

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void ModulePinsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        Q_EMIT updateText(QString("Pins (%1)").arg(newNumberPorts));
    }

    void ModulePinsTree::appendMultiSelectionEntries(QMenu& menu, int modId)
    {
        QList<TreeItem*> selectedPins;
        std::pair<bool, int> sameGroup;
        bool onlyPins;
        std::tie(selectedPins, sameGroup, onlyPins) = getSelectedPins();
        if (selectedPins.size() > 1)
        {
            menu.addAction("Add objects to new pin group", [this, selectedPins, modId]() {
                InputDialog ipd("Pingroup name", "New pingroup name", "ExampleName");
                if (ipd.exec() == QDialog::Accepted && !ipd.textValue().isEmpty())
                {
                    QSet<u32> pins;
                    auto mod = gNetlist->get_module_by_id(modId);
                    for (auto item : selectedPins)
                    {
                        auto* pin = mod->get_pin_by_id(mPortModel->getIdOfItem(item));
                        if (pin == nullptr)
                            return;
                        pins.insert(pin->get_id());
                    }
                    UserActionCompound* act = new UserActionCompound;
                    act->setUseCreatedObject();
                    ActionCreateObject* actCreate = new ActionCreateObject(UserActionObjectType::PinGroup, ipd.textValue());
                    actCreate->setParentObject(UserActionObject(modId, UserActionObjectType::Module));
                    ActionAddItemsToObject* actAdd = new ActionAddItemsToObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), pins);
                    actAdd->setUsedInCreateContext();
                    act->addAction(actCreate);
                    act->addAction(actAdd);
                    act->exec();
                }
            });
        }
    }

    std::tuple<QList<TreeItem*>, std::pair<bool, int>, bool> ModulePinsTree::getSelectedPins()
    {
        QList<TreeItem*> selectedPins;           //ordered
        QSet<TreeItem*> alreadyProcessedPins;    //only for performance purposes
        bool sameGroup = true;
        bool onlyPins  = true;
        int groupId    = -1;
        for (auto index : selectionModel()->selectedRows())
        {
            TreeItem* item = mPortModel->getItemFromIndex(index);
            auto itemType  = mPortModel->getTypeOfItem(item);
            if (itemType == ModulePinsTreeModel::itemType::pin)
            {
                if (!alreadyProcessedPins.contains(item))
                {
                    selectedPins.append(item);
                    alreadyProcessedPins.insert(item);
                }
            }
            else if (itemType == ModulePinsTreeModel::itemType::group)
            {
                onlyPins = false;
                for (auto pinItem : item->getChildren())
                {
                    if (!alreadyProcessedPins.contains(pinItem))
                    {
                        selectedPins.append(pinItem);
                        alreadyProcessedPins.insert(pinItem);
                    }
                }
            }
        }
        //Check if all pins are from the same group (if no pin is selected it is from the same group)
        if (!selectedPins.isEmpty())
        {
            auto mod       = gNetlist->get_module_by_id(mModuleID);
            auto* firstPin = mod->get_pin_by_id(mPortModel->getIdOfItem(selectedPins.front()));
            groupId        = firstPin->get_group().first->get_id();
            for (auto pinTreeItem : selectedPins)
            {
                auto pin = mod->get_pin_by_id(mPortModel->getIdOfItem(pinTreeItem));
                if (groupId != (int)pin->get_group().first->get_id())
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
