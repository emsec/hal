#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"

#include "gui/basic_tree_model/tree_item.h"
#include "gui/gui_globals.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_reorder_object.h"
#include "gui/user_action/user_action_compound.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/enums.h"

#include <QDebug>
#include <QMimeData>

namespace hal
{
    ModulePinsTreeModel::ModulePinsTreeModel(QObject* parent) : BaseTreeModel(parent), mIgnoreEventsFlag(false)
    {
        setHeaderLabels(QList<QVariant>() << "Name"
                                          << "Direction"
                                          << "Type"
                                          << "Connected Net");
        setModule(gNetlist->get_module_by_id(1));

        //connections
        connect(gNetlistRelay, &NetlistRelay::modulePortsChanged, this, &ModulePinsTreeModel::handleModulePortsChanged);
    }

    ModulePinsTreeModel::~ModulePinsTreeModel()
    {
        delete mRootItem;
    }

    Qt::ItemFlags ModulePinsTreeModel::flags(const QModelIndex& index) const
    {
//        Qt::ItemFlags defaultFlags = BaseTreeModel::flags(index);
//        TreeItem* item             = index.isValid() ? getItemFromIndex(index) : nullptr;
//        if (item)
//        {
//            //get parent, must be a pingroup item and not the root (not allowed to drag from external group, but maybe later)
//            TreeItem* parentItem = item->getParent();
//            itemType type        = getTypeOfItem(item);
//            if (type == itemType::portMultiBit)
//                return defaultFlags | Qt::ItemIsDropEnabled;
//            else if (type == itemType::pin)                     // && parentItem != mRootItem)//only case that should be possible
//                return defaultFlags | Qt::ItemIsDragEnabled;    // | Qt::ItemIsDropEnabled;
//            if (parentItem == mRootItem && type == itemType::pin)
//                return defaultFlags;
//        }
        // valid-check must be ommitted when a drop between pingroups is desired, all checks are performed in canDropMimeData
        return BaseTreeModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
//        if(item)
//        {
//            // everything can be dragged, but wether it can be dropped on or not depends on the situation
//            // -> a pin cannot be dropped onto its parent-group (but it can be dropped within its group),
//            // and a pingroup cannot be dropped onto (or between) a pin. -> what is currently dragged can only
//            // be checked in the "canDropMimeData" function, ItemIsDropEnabled cannot be set since its conditional on the dragged item
//            return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

//        }
//        return defaultFlags;
    }

    QStringList ModulePinsTreeModel::mimeTypes() const
    {
        QStringList types;
        types << "pintreemodel/item";
        return types;
    }

    QMimeData* ModulePinsTreeModel::mimeData(const QModelIndexList& indexes) const
    {
        if (indexes.size() != 4)    //columncount, only 1 item is allowed
            return new QMimeData();

        QMimeData* data = new QMimeData();
        auto item       = getItemFromIndex(indexes.at(0));
        QByteArray encodedData;
        QDataStream stream(&encodedData, QIODevice::WriteOnly);
        QString type = getTypeOfItem(item) == itemType::pin ? "pin" : "group";
        stream << type << getIdOfItem(item);
        data->setText(item->getData(sNameColumn).toString());
        data->setData("pintreemodel/item", encodedData);
        return data;
    }
    bool ModulePinsTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
    {
        Q_UNUSED(action)
        Q_UNUSED(column)

        QString type;
        int id;
        QByteArray encItem = data->data("pintreemodel/item");
        QDataStream dataStream(&encItem, QIODevice::ReadOnly);
        dataStream >> type >> id;

        auto droppedItem = (type == "group") ? mIdToGroupItem.value(id) : mIdToPinItem.value(id);
        auto droppedParentItem = droppedItem->getParent();
        auto parentItem = getItemFromIndex(parent);

        // perhaps helper functions?
        // 1. group on group (between group)
        // 2. pin on group
        // 3. pin between groups
        // 4. pin between pins

        if(type == "group")
        {
            if(!parentItem)
                qDebug() << "group was dropped between groups... with row: " << row; //check in canDropMine if its not an adjacent row?
            else{
                qDebug() << "group was dropped on a group?";
                InputDialog ipd("Name of new group", "Name of new group:", parentItem->getData(sNameColumn).toString());
                if(ipd.exec() == QDialog::Rejected) return false;
                mIgnoreEventsFlag = true;
                auto mod = gNetlist->get_module_by_id(mModuleId);
                auto group1 = mod->get_pin_group_by_id(id);
                auto group2 = mod->get_pin_group_by_id(getIdOfItem(parentItem));
                for(auto pin: group1->get_pins())
                    mod->assign_pin_to_group(group2, pin, false);
                mod->delete_pin_group(group1);
                mod->set_pin_group_name(group2, ipd.textValue().toStdString());
                mIgnoreEventsFlag = false;
                setModule(mod);
                return true;
            }
        }
        else
        {
            if(!parentItem)
                qDebug() << "pin was dropped between groups on row " << row;
            else if(row != -1)
                //qDebug() << "pin was dropped between pins";
                dndPinBetweenPin(droppedItem, parentItem, row);
            else
                qDebug() << "pin was dropped on a group...";
        }
    }

//    bool ModulePinsTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
//    {
//        Q_UNUSED(action)
//        Q_UNUSED(column)

//        QString type;
//        int id;
//        QByteArray encItem = data->data("pintreemodel/item");
//        QDataStream dataStream(&encItem, QIODevice::ReadOnly);
//        dataStream >> type;
//        dataStream >> id;

//        auto droppedItem       = mIdToPinItem.value(id);
//        auto droppedParentItem = droppedItem->getParent();
//        auto mod               = gNetlist->get_module_by_id(mModuleId);
//        auto droppedPin        = mod->get_pin_by_id(id);
//        if (droppedPin == nullptr)
//            return false;
//        auto ownRow = droppedItem->getOwnRow();
//        TreeItem* newItem =
//            new TreeItem(QList<QVariant>() << droppedItem->getData(sNameColumn) << droppedItem->getData(sDirectionColumn) << droppedItem->getData(sTypeColumn) << droppedItem->getData(sNetColumn));
//        newItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));    //currently only pins can be dragged, so its a pin
//        newItem->setAdditionalData(keyId, id);

//        if (row != -1)    //between items
//        {
//            auto onDroppedParentItem = parent.isValid() ? getItemFromIndex(parent) : mRootItem;
//            bool bottomEdge          = row == onDroppedParentItem->getChildCount();
//            auto onDroppedItem       = bottomEdge ? onDroppedParentItem->getChild(row - 1) : onDroppedParentItem->getChild(row);
//            auto* onDroppedPin       = mod->get_pin_by_id(getIdOfItem(onDroppedItem));
//            auto desiredIndex        = onDroppedPin->get_group().second;

//            //same-parent (parent != root): move withing same group
//            if (onDroppedParentItem == droppedParentItem && onDroppedParentItem != mRootItem)
//            {
//                if (ownRow == row || ownRow + 1 == row)
//                    return false;

//                removeItem(droppedItem);
//                mIgnoreEventsFlag = true;
//                if (bottomEdge)
//                {
//                    insertItem(newItem, droppedParentItem, row - 1);
//                    ActionReorderObject* reorderObj = new ActionReorderObject(desiredIndex);
//                    reorderObj->setObject(UserActionObject(droppedPin->get_id(), UserActionObjectType::Pin));
//                    reorderObj->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
//                    reorderObj->exec();
//                }
//                else
//                {
//                    if (ownRow < row)
//                    {
//                        insertItem(newItem, droppedParentItem, row - 1);
//                        desiredIndex--;
//                    }
//                    else
//                        insertItem(newItem, droppedParentItem, row);
//                    ActionReorderObject* reorderObj = new ActionReorderObject(desiredIndex);
//                    reorderObj->setObject(UserActionObject(droppedPin->get_id(), UserActionObjectType::Pin));
//                    reorderObj->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
//                    reorderObj->exec();
//                }
//                mIgnoreEventsFlag = false;
//                return true;
//            }

//            //different parents v1 (dropped's parent = mRoot, onDropped's parent=group)
//            //+ different parents v2(droppedItem's parent != root, ondropped's != root)
//            if ((droppedParentItem == mRootItem && onDroppedParentItem != mRootItem) || (droppedParentItem != mRootItem && onDroppedParentItem != mRootItem))
//            {
//                auto* pinGroup = mod->get_pin_group_by_id(getIdOfItem(onDroppedParentItem));
//                if (pinGroup == nullptr)
//                    return false;

//                mIgnoreEventsFlag        = true;
//                UserActionCompound* comp = new UserActionCompound;

//                if (droppedParentItem != mRootItem && onDroppedParentItem != mRootItem)
//                {
//                    //for undo action, reordering can only be performed when not dragging from the top level
//                    ActionReorderObject* reordActHack = new ActionReorderObject(droppedPin->get_group().second);
//                    reordActHack->setObject(UserActionObject(droppedPin->get_id(), UserActionObjectType::Pin));
//                    reordActHack->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
//                    comp->addAction(reordActHack);
//                }
//                ActionAddItemsToObject* addAct = new ActionAddItemsToObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), QSet<u32>() << droppedPin->get_id());
//                addAct->setObject(UserActionObject(pinGroup->get_id(), UserActionObjectType::PinGroup));
//                addAct->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
//                ActionReorderObject* reordAct = new ActionReorderObject(bottomEdge ? desiredIndex + 1 : desiredIndex);
//                reordAct->setObject(UserActionObject(droppedPin->get_id(), UserActionObjectType::Pin));
//                reordAct->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
//                comp->addAction(addAct);
//                comp->addAction(reordAct);
//                bool ret          = comp->exec();
//                mIgnoreEventsFlag = false;
//                if (ret)
//                {
//                    removeItem(droppedItem);
//                    insertItem(newItem, onDroppedParentItem, row);
//                }
//                else
//                    setModule(gNetlist->get_module_by_id(mModuleId));
//            }
//        }
//        else    // on item
//        {
//            auto onDroppedItem   = getItemFromIndex(parent);
//            auto* onDroppedGroup = mod->get_pin_group_by_id(getIdOfItem(onDroppedItem));
//            if (onDroppedGroup == nullptr)
//                return false;
//            //on group (dropped parent = mRoot)
//            //if(droppedParentItem == mRootItem)//item which is dropped
//            if (droppedParentItem != onDroppedItem)
//            {
//                mIgnoreEventsFlag = true;
//                //int ret = mod->assign_pin_to_group(onDroppedGroup, droppedPin).is_ok();
//                ActionAddItemsToObject* addAct = new ActionAddItemsToObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), QSet<u32>() << droppedPin->get_id());
//                addAct->setObject(UserActionObject(onDroppedGroup->get_id(), UserActionObjectType::PinGroup));
//                addAct->setParentObject(UserActionObject(mod->get_id(), UserActionObjectType::Module));
//                bool ret = addAct->exec();
//                if (ret)
//                {
//                    removeItem(droppedItem);
//                    insertItem(newItem, onDroppedItem, onDroppedItem->getChildCount());
//                    mIgnoreEventsFlag = false;
//                    return true;
//                }
//                mIgnoreEventsFlag = false;
//                return false;
//            }
//        }
//        return false;
//    }

    bool ModulePinsTreeModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
    {
        Q_UNUSED(action)
        if(!data->formats().contains("pintreemodel/item")) return false;

        QString type; int id;
        auto encItem = data->data("pintreemodel/item");
        QDataStream dataStream(&encItem, QIODevice::ReadOnly);
        dataStream >> type >> id;
        auto parentItem = getItemFromIndex(parent);
        qDebug() << "type: " << type << ", id" << id << ", row: " << row;

        // construct a "drop-matrix" here, but only 4 things are NOT allowed (so check for these):
        // 1: drop a pin on its OWN parent
        // 3: drop a pingroup on/between pins
        // 4: drop an item on itself
        if(type == "group")
        {
            if(parentItem)
            {
                auto item = mIdToGroupItem[id];
                auto type = getTypeOfItem(parentItem);
                if(type == itemType::pin || (type == itemType::group && row != -1) || item == parentItem)
                    return false;
            }
        }
        if(type == "pin")
        {
            // perhaps check here that a pin can only be dropped between groups if its own group has size > 1?
            // otherwise it does not make much sense...perhaps change this check
            auto item = mIdToPinItem[id];
            if((!parentItem && item->getParent()->getChildCount() == 1) || (item->getParent() == parentItem && row == -1) || item == parentItem
                || (parentItem && (getTypeOfItem(parentItem) == itemType::pin)))
                return false;
//            if(parentItem)
//            {
//                if(item->getParent() == parentItem || item == parentItem)
//                    return false;
//            }
        }
        return true;
    }

    void ModulePinsTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleId = -1;
        mNameToTreeItem.clear();
        mIdToGroupItem.clear();
        mIdToPinItem.clear();
    }

    void ModulePinsTreeModel::setModule(Module* m)
    {
        clear();
        mModuleId = m->get_id();
        beginResetModel();

        for(PinGroup<ModulePin>* pinGroup : m->get_pin_groups())
        {
            if(pinGroup->empty())
                continue;

            auto pinGroupName = QString::fromStdString(pinGroup->get_name());
            auto pinGroupDirection = QString::fromStdString(enum_to_string((pinGroup->get_direction())));
            auto pinGroupType = QString::fromStdString(enum_to_string(pinGroup->get_type()));

            TreeItem* pinGroupItem = new TreeItem(QList<QVariant>() << pinGroupName << pinGroupDirection << pinGroupType << "");
            pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::group)); // port multi bit
            pinGroupItem->setAdditionalData(keyId, pinGroup->get_id());
            mIdToGroupItem.insert(pinGroup->get_id(), pinGroupItem);
            for(ModulePin* pin : pinGroup->get_pins())
            {
                TreeItem* pinItem = new TreeItem(QList<QVariant>() << QString::fromStdString(pin->get_name())
                                                 << QString::fromStdString(enum_to_string(pin->get_direction()))
                                                 << QString::fromStdString(enum_to_string(pin->get_type()))
                                                 << QString::fromStdString(pin->get_net()->get_name()));
                pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
                pinItem->setAdditionalData(keyId, pin->get_id());
                pinGroupItem->appendChild(pinItem);
                mNameToTreeItem.insert(QString::fromStdString(pin->get_name()), pinItem);
                mIdToPinItem.insert(pin->get_id(), pinItem);
            }
            mNameToTreeItem.insert(pinGroupName, pinGroupItem);
            mRootItem->appendChild(pinGroupItem);
        }

//        for (PinGroup<ModulePin>* pinGroup : m->get_pin_groups())
//        {
//            //ignore empty pingroups
//            if (pinGroup->empty())
//                continue;


//            ModulePin* firstPin = pinGroup->get_pins().front();
//            QString pinGroupName;
//            QString pinGroupDirection = QString::fromStdString(enum_to_string(firstPin->get_direction()));
//            QString pinGroupType      = QString::fromStdString(enum_to_string(firstPin->get_type()));
//            if (pinGroup->size() == 1)
//            {
//                pinGroupName = QString::fromStdString(firstPin->get_name());
//            }
//            else
//            {
//                pinGroupName = QString::fromStdString(pinGroup->get_name());
//            }

//            TreeItem* pinGroupItem = new TreeItem(QList<QVariant>() << pinGroupName << pinGroupDirection << pinGroupType << "");

//            if (pinGroup->size() == 1)
//            {
//                pinGroupItem->setDataAtIndex(sNetColumn, QString::fromStdString(firstPin->get_net()->get_name()));
//                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
//                //since a single-pin pingroup represents the pin itself, take the pinid
//                pinGroupItem->setAdditionalData(keyId, firstPin->get_id());
//                mIdToPinItem.insert(firstPin->get_id(), pinGroupItem);
//            }
//            else
//            {
//                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portMultiBit));
//                pinGroupItem->setAdditionalData(keyId, pinGroup->get_id());
//                mIdToGroupItem.insert(pinGroup->get_id(), pinGroupItem);
//                for (ModulePin* pin : pinGroup->get_pins())
//                {
//                    TreeItem* pinItem =
//                        new TreeItem(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinGroupDirection << pinGroupType << QString::fromStdString(pin->get_net()->get_name()));
//                    pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
//                    pinItem->setAdditionalData(keyId, pin->get_id());
//                    pinGroupItem->appendChild(pinItem);
//                    mNameToTreeItem.insert(QString::fromStdString(pin->get_name()), pinItem);
//                    mIdToPinItem.insert(pin->get_id(), pinItem);
//                }
//            }
//            mRootItem->appendChild(pinGroupItem);
//            mNameToTreeItem.insert(pinGroupName, pinGroupItem);
//        }
        endResetModel();

        Q_EMIT numberOfPortsChanged(m->get_pins().size());
    }

    Net* ModulePinsTreeModel::getNetFromItem(TreeItem* item)
    {
        if (mModuleId == -1)    //no current module = no represented net
            return nullptr;

        itemType type = getTypeOfItem(item);
        if (type == itemType::portMultiBit)
            return nullptr;

        Module* m = gNetlist->get_module_by_id(mModuleId);
        if (!m)
            return nullptr;

        //std::string name = item->getData(sNameColumn).toString().toStdString();
        if (auto* pin = m->get_pin_by_id(getIdOfItem(item)); pin != nullptr)
        {
            return pin->get_net();
        }

        return nullptr;
    }

    int ModulePinsTreeModel::getRepresentedModuleId()
    {
        return mModuleId;
    }

    ModulePinsTreeModel::itemType ModulePinsTreeModel::getTypeOfItem(TreeItem* item) const
    {
        return item->getAdditionalData(keyType).value<itemType>();
    }

    int ModulePinsTreeModel::getIdOfItem(TreeItem* item) const
    {
        return item->getAdditionalData(keyId).toInt();
    }

    void ModulePinsTreeModel::handleModulePortsChanged(Module* m)
    {
        if ((int)m->get_id() == mModuleId)
        {
            if (!mIgnoreEventsFlag)
                setModule(m);
        }
    }

    void ModulePinsTreeModel::removeItem(TreeItem* item)
    {
        beginRemoveRows(parent(getIndexFromItem(item)), item->getOwnRow(), item->getOwnRow());
        item->getParent()->removeChild(item);
        endRemoveRows();
        //mNameToTreeItem.remove(item->getData(sNameColumn).toString());
        //for now, only ids of pin-items (since these functions are only relevant for dnd)
        mIdToPinItem.remove(getIdOfItem(item));
        delete item;
    }

    bool ModulePinsTreeModel::dndPinBetweenPin(TreeItem *droppedPin, TreeItem *onDroppedParent, int row)
    {
        if(droppedPin->getParent() == onDroppedParent)
            qDebug() << "pin dropped in same group";
        else
            qDebug() << "pin dropped in another group";

    }

    void ModulePinsTreeModel::insertItem(TreeItem* item, TreeItem* parent, int index)
    {
        beginInsertRows(getIndexFromItem(parent), index, index);
        parent->insertChild(index, item);
        endInsertRows();
        //mNameToTreeItem.insert(item->getData(sNameColumn).toString(), item);
        mIdToPinItem.insert(getIdOfItem(item), item);
    }
}    // namespace hal
