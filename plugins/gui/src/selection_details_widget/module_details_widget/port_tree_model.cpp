#include "gui/selection_details_widget/module_details_widget/port_tree_model.h"

#include "gui/basic_tree_model/tree_item.h"
#include "gui/gui_globals.h"
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

    Qt::ItemFlags ModulePinsTreeModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags defaultFlags = BaseTreeModel::flags(index);
        TreeItem* item = index.isValid() ? getItemFromIndex(index) : nullptr;
        if(item)
        {
            //get parent, must be a pingroup item and not the root (not allowed to drag from external group, but maybe later)
            TreeItem* parentItem = item->getParent();
            itemType type = getTypeOfItem(item);
            if(type == itemType::portMultiBit)
                return defaultFlags | Qt::ItemIsDropEnabled;
            else if(type == itemType::pin && parentItem != mRootItem)//only case that should be possible
                return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
            if(parentItem == mRootItem && type == itemType::pin)
                return defaultFlags;
        }
        return defaultFlags;
    }

    QStringList ModulePinsTreeModel::mimeTypes() const
    {
        QStringList types;
        types << "pintreemodel/pin";
        return  types;
    }

    QMimeData *ModulePinsTreeModel::mimeData(const QModelIndexList &indexes) const
    {
        if(indexes.size() != 4) //columncount, only 1 item is allowed
            return new QMimeData();

        qDebug() << "started dragging";
        QMimeData* data = new QMimeData();
        auto item = getItemFromIndex(indexes.at(0));
        qDebug() << item->getData(sNameColumn).toString();
        data->setText(item->getData(sNameColumn).toString());
        data->setData("pintreemodel/pin", QByteArray());
        return data;
    }

    bool ModulePinsTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        //ROW IS USED FOR BETWEEN ITEMS,YES!!!
        qDebug() << "DROPPED A BOI with info:";
        qDebug() << "Row: " << row;
        qDebug() << "Column: " << column;

        //move_pin_within_group
        //index through get_group of the pin class
        if(row != -1)//inserted between items
        {
            auto droppedItem = mNameToTreeItem.value(data->text());
            auto parentItem = droppedItem->getParent();
            auto mod = gNetlist->get_module_by_id(mModuleId);
            auto ownRow = droppedItem->getOwnRow();
            //check if dropped on adjacent positions, if yes, ignore it
            if(ownRow == row || ownRow+1 == row)
                return false;

            //edge case (literally bottom edge)
            if(row == droppedItem->getParent()->getChildCount()+1)
            {

            }
            else
            {
                auto onDroppedItem = droppedItem->getParent()->getChild(row);//"old" item (to get the old index)
                auto onDroppedPin = mod->get_pin(onDroppedItem->getData(sNameColumn).toString().toStdString());
                auto droppedPin = mod->get_pin(droppedItem->getData(sNameColumn).toString().toStdString());
                auto desiredIndex = onDroppedPin->get_group().second;
                TreeItem* newItem = new TreeItem(QList<QVariant>() << droppedItem->getData(sNameColumn) << droppedItem->getData(sDirectionColumn)
                                                 << droppedItem->getData(sTypeColumn) << droppedItem->getData(sNetColumn));
                newItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));

                //remove old item:
                removeItem(droppedItem);
                //insert new item based in the oldIndex
                if(ownRow < row){
                    insertItem(newItem, parentItem, row-1);
                    desiredIndex--;
                }
                else
                    insertItem(newItem, parentItem, row);

                mIgnoreNextPinsChanged = true;
                mod->move_pin_within_group(droppedPin->get_group().first, droppedPin, desiredIndex);
                return true;
            }

        }

        if(parent.isValid())
        {
            auto item = getItemFromIndex(parent);
            qDebug() << "parent-name: " << item->getData(sNameColumn).toString();
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
            }
            else
            {
                pinGroupItem->setAdditionalData(keyType, QVariant::fromValue(itemType::portMultiBit));
                for (ModulePin* pin : pinGroup->get_pins())
                {
                    TreeItem* pinItem = new TreeItem(QList<QVariant>() << QString::fromStdString(pin->get_name()) << pinGroupDirection << pinGroupType << QString::fromStdString(pin->get_net()->get_name()));
                    pinItem->setAdditionalData(keyType, QVariant::fromValue(itemType::pin));
                    pinGroupItem->appendChild(pinItem);
                    mNameToTreeItem.insert(QString::fromStdString(pin->get_name()), pinItem);
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

        std::string name = item->getData(sNameColumn).toString().toStdString();
        if (ModulePin* pin = m->get_pin(name); pin != nullptr)
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

    void ModulePinsTreeModel::handleModulePortsChanged(Module *m)
    {
        if ((int)m->get_id() == mModuleId)
        {
            if(mIgnoreNextPinsChanged)
                mIgnoreNextPinsChanged = false;
            else
                setModule(m);
        }
    }

    void ModulePinsTreeModel::removeItem(TreeItem *item)
    {
        beginRemoveRows(parent(getIndexFromItem(item)), item->getOwnRow(), item->getOwnRow());
        item->getParent()->removeChild(item);
        endRemoveRows();
        mNameToTreeItem.remove(item->getData(sNameColumn).toString());
        delete item;
    }

    void ModulePinsTreeModel::insertItem(TreeItem *item, TreeItem* parent, int index)
    {
        beginInsertRows(getIndexFromItem(parent), index, index);
        parent->insertChild(index, item);
        endInsertRows();
        mNameToTreeItem.insert(item->getData(sNameColumn).toString(), item);
    }
}    // namespace hal
