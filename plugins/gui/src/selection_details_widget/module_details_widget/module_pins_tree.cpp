#include "gui/selection_details_widget/module_details_widget/module_pins_tree.h"

#include "gui/gui_globals.h"
#include "gui/input_dialog/combobox_dialog.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/input_dialog/pingroup_selector_dialog.h"
#include "gui/python/py_code_provider.h"
#include "gui/selection_details_widget/module_details_widget/module_pins_tree_model.h"
#include "gui/user_action/action_pingroup.h"
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

        mPortModel->clear();
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
        ModulePinsTreeItem* clickedItem          =  static_cast<ModulePinsTreeItem*>(mPortModel->getItemFromIndex(clickedIndex));
        Net* n                             = mPortModel->getNetFromItem(clickedItem);
        QString name                       = clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString();
        u32 modId                          = mPortModel->getRepresentedModuleId();
        auto mod                           = gNetlist->get_module_by_id(modId);
        QList<BaseTreeItem*> selectedPins;
        std::pair<bool, int> sameGroup;
        bool onlyPins;
        std::tie(selectedPins, sameGroup, onlyPins) = getSelectedPins();
        int itemId                                  = clickedItem->id();
        QMenu menu;

        //shared plaintext entries: NAME, DIRECTION, TYPE (shared with pins and groups)
        menu.addAction("Name to clipboard", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sNameColumn).toString()); });
        menu.addAction("Direction to clipboard", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sDirectionColumn).toString()); });
        menu.addAction("Type to clipboard", [clickedItem]() { QApplication::clipboard()->setText(clickedItem->getData(ModulePinsTreeModel::sTypeColumn).toString()); });

        menu.addSection("Misc");

        //shared context menu entry to add to existing groups
        bool addToExistingActionPossible = true;
//        for (auto pingroup : mod->get_pin_groups())
//        {
//            if (pingroup->size() > 1)    //at least one pingroup should have at least 2 items
//            {
//                addToExistingActionPossible = true;
//                break;
//            }
//        }
        if (addToExistingActionPossible)
        {
            menu.addAction("Add selection to existing pin group", [selectedPins, mod]() {
                PingroupSelectorDialog psd("Pingroup selector", "Select pingroup", mod, false);
                if (psd.exec() == QDialog::Accepted)
                {
                    QList<u32> pins;
                    auto* pinGroup = mod->get_pin_group_by_id(psd.getSelectedGroupId());
                    if (pinGroup == nullptr)
                        return;
                    for (auto item : selectedPins)
                    {
                        auto* pin = mod->get_pin_by_id(static_cast<ModulePinsTreeItem*>(item)->id());
                        if (pin == nullptr)
                            return;
                        pins.append(pin->get_id());
                    }
                    ActionPingroup* act = ActionPingroup::addPinsToExistingGroup(mod,pinGroup->get_id(),pins);
                    if (act) act->exec();
                }
            });
        }

        if (clickedItem->itemType() == ModulePinsTreeItem::Group)    //group specific context, own helper function? (returns at the end)
        {
            menu.addAction("Change name", [itemId, mod, name]() {
                PinGroup<ModulePin>*  pg = mod->get_pin_group_by_id(itemId);
                if (pg == nullptr) return;

                InputDialog ipd("Change pin group name", "New group name", name);
                if (ipd.exec() == QDialog::Accepted)
                {
                    if (ipd.textValue().isEmpty())
                        return;

                    ActionPingroup* act = new ActionPingroup(PinActionType::GroupRename,itemId,ipd.textValue());
                    act->setObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                    act->exec();
                }
            });

            menu.addAction("Change type", [itemId, mod, name]() {
                PinGroup<ModulePin>* pg = mod->get_pin_group_by_id(itemId);
                if (pg == nullptr) return;

                QStringList types;
                for (auto const& [k, v] : EnumStrings<PinType>::data)
                    types << QString::fromStdString(v);

                ComboboxDialog cbd("Change type", "Select type for pin group " + name, types);

                if (cbd.exec() == QDialog::Accepted)
                {
                    PinType ptype = enum_from_string<PinType>(cbd.textValue().toStdString(),PinType::none);
                    ActionPingroup* act = ActionPingroup::changePinGroupType(mod,itemId,(int)ptype);
                    if (act) act->exec();
                }
            });
            menu.addAction("Toggle ascending/descending", [itemId, mod](){
                ActionPingroup* act = ActionPingroup::toggleAscendingGroup(mod, itemId);
                if (act) act->exec();
            });
            menu.addAction("Autmatically rename pins", [itemId, mod](){
                ActionPingroup* act = ActionPingroup::automaticallyRenamePins(mod, itemId);
                if (act) act->exec();
            });
            menu.addAction("Delete pin group", [itemId, mod]() {
                auto* pinGroup = mod->get_pin_group_by_id(itemId);
                if (pinGroup != nullptr)
                {
                    ActionPingroup* act = ActionPingroup::deletePinGroup(mod,itemId);
                    if (act) act->exec();
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
                        ActionPingroup* act = new ActionPingroup(PinActionType::PinRename,pin->get_id(),ipd.textValue());
                        act->setObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
                        act->exec();
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
                    PinType ptype = enum_from_string<PinType>(cbd.textValue().toStdString(),PinType::none);

                    ActionPingroup* act = new ActionPingroup(PinActionType::PinTypeChange,pin->get_id(),"",(int)ptype);
                    act->setObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
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
            menu.addAction("Remove selection from group", [selectedPins, mod /*, sameGroup*/]() {
                QList<u32> pins;
                for (auto item : selectedPins)
                    pins.append(static_cast<ModulePinsTreeItem*>(item)->id());

                ActionPingroup* act = ActionPingroup::removePinsFromGroup(mod, pins);
                if (act) act->exec();
            });
        }

        //multi-selection (part of misc)
        if (selectionModel()->selectedRows().size() > 1)
            appendMultiSelectionEntries(menu, modId);

        menu.addSection("Python");
        if(clickedItem->itemType()==ModulePinsTreeItem::Pin)
            menu.addAction(QIcon(":/icons/python"), "Get pin", [modId, itemId]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinById(modId, itemId)); });
        else
            menu.addAction(QIcon(":/icons/python"), "Get group", [modId, itemId]() { QApplication::clipboard()->setText(PyCodeProvider::pyCodeModulePinGroup(modId, itemId)); });

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void ModulePinsTree::handleNumberOfPortsChanged(int newNumberPorts)
    {
        Q_EMIT updateText(QString("Pins (%1)").arg(newNumberPorts));
    }

    void ModulePinsTree::appendMultiSelectionEntries(QMenu& menu, int modId)
    {
        QList<BaseTreeItem*> selectedPins;
        std::pair<bool, int> sameGroup;
        bool onlyPins;
        std::tie(selectedPins, sameGroup, onlyPins) = getSelectedPins();
        if (selectedPins.size() > 1)
        {
            menu.addAction("Add objects to new pin group", [selectedPins, modId]() {
                InputDialog ipd("Pingroup name", "New pingroup name", "ExampleName");
                if (ipd.exec() == QDialog::Accepted && !ipd.textValue().isEmpty())
                {
                    QList<u32> pins;
                    Module* mod = gNetlist->get_module_by_id(modId);
                    for (auto item : selectedPins)
                    {
                        auto* pin = mod->get_pin_by_id(static_cast<ModulePinsTreeItem*>(item)->id());
                        if (pin == nullptr)
                            return;
                        pins.append(pin->get_id());
                    }

                    ActionPingroup* act = ActionPingroup::addPinsToNewGroup(mod,ipd.textValue(),pins);
                    if (act) act->exec();
                }
            });
        }
    }

    std::tuple<QList<BaseTreeItem*>, std::pair<bool, int>, bool> ModulePinsTree::getSelectedPins()
    {
        QList<BaseTreeItem*> selectedPins;           //ordered
        QSet<BaseTreeItem*> alreadyProcessedPins;    //only for performance purposes
        bool sameGroup = true;
        bool onlyPins  = true;
        int groupId    = -1;
        for (auto index : selectionModel()->selectedRows())
        {
            ModulePinsTreeItem* item =  static_cast<ModulePinsTreeItem*>(mPortModel->getItemFromIndex(index));
            if (item->itemType() == ModulePinsTreeItem::Pin)
            {
                if (!alreadyProcessedPins.contains(item))
                {
                    selectedPins.append(item);
                    alreadyProcessedPins.insert(item);
                }
            }
            else if (item->itemType() == ModulePinsTreeItem::Group)
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
            auto* firstPin = mod->get_pin_by_id(static_cast<ModulePinsTreeItem*>(selectedPins.front())->id());
            groupId        = firstPin->get_group().first->get_id();
            for (auto pinTreeItem : selectedPins)
            {
                auto pin = mod->get_pin_by_id(static_cast<ModulePinsTreeItem*>(pinTreeItem)->id());
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
