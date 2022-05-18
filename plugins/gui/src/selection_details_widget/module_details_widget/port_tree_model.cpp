#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"

#include "gui/basic_tree_model/tree_item.h"
#include "gui/gui_globals.h"
#include "gui/user_action/action_reorder_object.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/enums.h"

#include <QMimeData>

namespace hal
{
    ModulePinsTreeModel::ModulePinsTreeModel(QObject* parent) : BaseTreeModel(parent), mIgnoreNextPinsChanged(false)
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
        Qt::ItemFlags defaultFlags = BaseTreeModel::flags(index);
        TreeItem* item             = index.isValid() ? getItemFromIndex(index) : nullptr;
        if (item)
        {
            //get parent, must be a pingroup item and not the root (not allowed to drag from external group, but maybe later)
            TreeItem* parentItem = item->getParent();
            itemType type        = getTypeOfItem(item);
            if (type == itemType::portMultiBit)
                return defaultFlags | Qt::ItemIsDropEnabled;
            else if (type == itemType::pin)                     // && parentItem != mRootItem)//only case that should be possible
                return defaultFlags | Qt::ItemIsDragEnabled;    // | Qt::ItemIsDropEnabled;
            if (parentItem == mRootItem && type == itemType::pin)
                return defaultFlags;
        }
        return defaultFlags;
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
        stream << QString("pin") << getIdOfItem(item);
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
        dataStream >> type;
        dataStream >> id;

        auto droppedItem       = mIdToPinItem.value(id);
        auto droppedParentItem = droppedItem->getParent();
        auto mod               = gNetlist->get_module_by_id(mModuleId);
        auto droppedPin        = mod->get_pin_by_id(id);
        if (droppedPin == nullptr)
        {
            return false;
        }
        auto ownRow = droppedItem->getOwnRow();
        TreeItem* newItem =
            new TreeItem(QList<QVariant>() << droppedItem->getData(sNameColumn) << droppedItem->getData(sDirectionColumn) << droppedItem->getData(sTypeColumn) << droppedItem->getData(sNetColumn));
        newItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));    //currently only pins can be dragged, so its a pin
        newItem->setAdditionalData(keyId, id);

        if (row != -1)    //between items
        {
            auto onDroppedParentItem = parent.isValid() ? getItemFromIndex(parent) : mRootItem;
            bool bottomEdge          = row == onDroppedParentItem->getChildCount();
            auto onDroppedItem       = bottomEdge ? onDroppedParentItem->getChild(row - 1) : onDroppedParentItem->getChild(row);
            auto onDroppedPin        = mod->get_pin_by_id(getIdOfItem(onDroppedItem));
            if (onDroppedPin == nullptr)
            {
                return false;
            }
            auto desiredIndex = onDroppedPin->get_group().second;

            //same-parent (parent != root): move withing same group
            if (onDroppedParentItem == droppedParentItem && onDroppedParentItem != mRootItem)
            {
                if (ownRow == row || ownRow + 1 == row)
                    return false;

                removeItem(droppedItem);
                mIgnoreNextPinsChanged = true;
                if (bottomEdge)
                {
                    insertItem(newItem, droppedParentItem, row - 1);
                    if (mod->move_pin_within_group(droppedPin->get_group().first, droppedPin, desiredIndex).is_error())
                    {
                        return false;
                    }
                }
                else
                {
                    if (ownRow < row)
                    {
                        insertItem(newItem, droppedParentItem, row - 1);
                        desiredIndex--;
                    }
                    else
                    {
                        insertItem(newItem, droppedParentItem, row);
                    }
                    if (mod->move_pin_within_group(droppedPin->get_group().first, droppedPin, desiredIndex).is_error())
                    {
                        return false;
                    }
                }
                return true;
            }

            //different parents v1 (dropped's parent = mRoot, onDropped's parent=group)
            if (droppedParentItem == mRootItem && onDroppedParentItem != mRootItem)
            {
                auto pinGroup = mod->get_pin_group_by_id(getIdOfItem(onDroppedParentItem));
                if (pinGroup == nullptr)
                {
                    return false;
                }
                mIgnoreNextPinsChanged = true;
                bool ret               = mod->assign_pin_to_group(pinGroup, droppedPin).is_ok();
                mIgnoreNextPinsChanged = false;    //if action above failed
                if (ret)
                {
                    mIgnoreNextPinsChanged = true;
                    ret                    = mod->move_pin_within_group(pinGroup, droppedPin, bottomEdge ? desiredIndex + 1 : desiredIndex).is_ok();
                    if (ret)
                    {
                        removeItem(droppedItem);
                        insertItem(newItem, onDroppedParentItem, row);
                        return true;
                    }
                    mIgnoreNextPinsChanged = false;
                    removeItem(droppedItem);
                    insertItem(newItem, onDroppedParentItem, onDroppedParentItem->getChildCount());
                    return false;
                }
                return false;
            }
            //different parents v2(droppedItem's parent != root, ondropped's != root)
        }
        else    // on item
        {
            auto onDroppedItem  = getItemFromIndex(parent);
            auto onDroppedGroup = mod->get_pin_group_by_id(getIdOfItem(onDroppedItem));
            if (onDroppedGroup == nullptr)
            {
                return false;
            }
            //on group (dropped parent = mRoot)
            if (droppedParentItem == mRootItem)
            {
                mIgnoreNextPinsChanged = true;
                int ret                = mod->assign_pin_to_group(onDroppedGroup, droppedPin).is_ok();
                if (ret)
                {
                    removeItem(droppedItem);
                    insertItem(newItem, onDroppedItem, onDroppedItem->getChildCount());
                    return true;
                }
                mIgnoreNextPinsChanged = false;
                return false;
            }
        }
        return false;
    }

    void ModulePinsTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleId = -1;
        mNameToTreeItem.clear();
    }

    void ModulePinsTreeModel::setModule(Module* m)
    {
        clear();
        mModuleId = m->get_id();
        beginResetModel();

        for (PinGroup<ModulePin>* pinGroup : m->get_pin_groups())
        {
            //ignore empty pingroups
            if (pinGroup->empty())
                continue;

            ModulePin* firstPin = pinGroup->get_pins().front();
            QString pinGroupName;
            QString pinGroupDirection = QString::fromStdString(enum_to_string(firstPin->get_direction()));
            QString pinGroupType      = QString::fromStdString(enum_to_string(firstPin->get_type()));
            if (pinGroup->size() == 1)
            {
                pinGroupName = QString::fromStdString(firstPin->get_name());
            }
            else
            {
                pinGroupName = QString::fromStdString(pinGroup->get_name());
            }

            TreeItem* pinGroupItem = new TreeItem(QList<QVariant>() << pinGroupName << pinGroupDirection << pinGroupType << "");

            if (pinGroup->size() == 1)
            {
                pinGroupItem->setDataAtIndex(sNetColumn, QString::fromStdString(firstPin->get_net()->get_name()));
                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
                //since a single-pin pingroup represents the pin itself, take the pinid
                pinGroupItem->setAdditionalData(keyId, firstPin->get_id());
                mIdToPinItem.insert(firstPin->get_id(), pinGroupItem);
            }
            else
            {
                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portMultiBit));
                pinGroupItem->setAdditionalData(keyId, pinGroup->get_id());
                mIdToGroupItem.insert(pinGroup->get_id(), pinGroupItem);
                for (ModulePin* pin : pinGroup->get_pins())
                {
                    TreeItem* pinItem =
                        new TreeItem(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinGroupDirection << pinGroupType << QString::fromStdString(pin->get_net()->get_name()));
                    pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
                    pinItem->setAdditionalData(keyId, pin->get_id());
                    pinGroupItem->appendChild(pinItem);
                    mNameToTreeItem.insert(QString::fromStdString(pin->get_name()), pinItem);
                    mIdToPinItem.insert(pin->get_id(), pinItem);
                }
            }
            mRootItem->appendChild(pinGroupItem);
            mNameToTreeItem.insert(pinGroupName, pinGroupItem);
        }
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
        if (const auto pin = m->get_pin_by_id(getIdOfItem(item)); pin != nullptr)
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
            if (mIgnoreNextPinsChanged)
                mIgnoreNextPinsChanged = false;
            else
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

    void ModulePinsTreeModel::insertItem(TreeItem* item, TreeItem* parent, int index)
    {
        beginInsertRows(getIndexFromItem(parent), index, index);
        parent->insertChild(index, item);
        endInsertRows();
        //mNameToTreeItem.insert(item->getData(sNameColumn).toString(), item);
        mIdToPinItem.insert(getIdOfItem(item), item);
    }
}    // namespace hal
