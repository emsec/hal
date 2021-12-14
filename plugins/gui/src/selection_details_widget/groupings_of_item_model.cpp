#include "gui/selection_details_widget/groupings_of_item_model.h"
#include "gui/content_manager/content_manager.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/gui_globals.h"

#include <algorithm>
#include <QApplication>

namespace hal {

    GroupingsOfItemModel::GroupingsOfItemModel(QObject* parent)
        : QAbstractTableModel(parent)
    {
        mGroupings.clear();
        mItemId = 0;

        // Event Connects
        connect(gNetlistRelay, &NetlistRelay::groupingRemoved, this, &GroupingsOfItemModel::handleGroupingRemoved);
        connect(gNetlistRelay, &NetlistRelay::groupingNameChanged, this, &GroupingsOfItemModel::handleGroupingNameChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingGateAssigned, this, &GroupingsOfItemModel::handleGroupingGateAssigned);
        connect(gNetlistRelay, &NetlistRelay::groupingGateRemoved, this, &GroupingsOfItemModel::handleGroupingGateRemoved);
        connect(gNetlistRelay, &NetlistRelay::groupingNetAssigned, this, &GroupingsOfItemModel::handleGroupingNetAssigned);
        connect(gNetlistRelay, &NetlistRelay::groupingNetRemoved, this, &GroupingsOfItemModel::handleGroupingNetRemoved);
        connect(gNetlistRelay, &NetlistRelay::groupingModuleAssigned, this, &GroupingsOfItemModel::handleGroupingModuleAssigned);
        connect(gNetlistRelay, &NetlistRelay::groupingModuleRemoved, this, &GroupingsOfItemModel::handleGroupingModuleRemoved);
        connect(gContentManager->getGroupingManagerWidget()->getModel(), &GroupingTableModel::groupingColorChanged, this, &GroupingsOfItemModel::handleGroupingColorChanged);
    }

    int GroupingsOfItemModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return 3;
    }

    int GroupingsOfItemModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mGroupings.size();
    }

    QVariant GroupingsOfItemModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
                case 0: return "Grouping Name";
                case 1: return "ID";
                case 2: return "Color";
                default: break;
            }
            return QVariant();
        }

        return section + 1;
    }

    QVariant GroupingsOfItemModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) return QVariant();
        const GroupingTableEntry& gte = mGroupings.at(index.row());

        switch (role) {
        case Qt::BackgroundRole:
            if (index.column()==2) return gte.color();
            return QVariant();
        case Qt::DisplayRole:
            switch (index.column()) {
            case 0:
                return gte.name();
            case 1:
                return gte.id();
            }
        default:
            break;
        }
        return QVariant();
    }

    bool GroupingsOfItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    }

    void GroupingsOfItemModel::setGate(Gate* gate)
    {
        if(gate == nullptr) return;
        mItemId = gate->get_id();
        mItemType = ItemType::Gate;

        QList<GroupingTableEntry> newGroupingList;
        // Temporary until items can be in multiple groupings
        Grouping* grp = gate->get_grouping();
        if(grp != nullptr){
            newGroupingList.append(GroupingTableEntry(grp));
        }
        layoutAboutToBeChanged();
        mGroupings = newGroupingList;
        layoutChanged();
    }

    void GroupingsOfItemModel::setModule(Module* module)
    {
        if(module == nullptr) return;
        mItemId = module->get_id();
        mItemType = ItemType::Module;

        QList<GroupingTableEntry> newGroupingList;
        // Temporary until items can be in multiple groupings
        Grouping* grp = module->get_grouping();
        if(grp != nullptr){
            newGroupingList.append(GroupingTableEntry(grp));
        }
        layoutAboutToBeChanged();
        mGroupings = newGroupingList;
        layoutChanged();
    }

    void GroupingsOfItemModel::setNet(Net* net)
    {
        if(net == nullptr) return;
        mItemId = net->get_id();
        mItemType = ItemType::Net;

        QList<GroupingTableEntry> newGroupingList;
        // Temporary until items can be in multiple groupings
        Grouping* grp = net->get_grouping();
        if(grp != nullptr){
            newGroupingList.append(GroupingTableEntry(grp));
        }
        layoutAboutToBeChanged();
        mGroupings = newGroupingList;
        layoutChanged();
    }

    void GroupingsOfItemModel::setGroupings(QList<Grouping*> groupingList)
    {
        mItemId = 0;
        mItemType = ItemType::None;

        QList<GroupingTableEntry> newGroupingList;
        for(Grouping* grp : groupingList)
        {
            if(grp == nullptr)
                continue;
            newGroupingList.append(GroupingTableEntry(grp));
            layoutAboutToBeChanged();
            mGroupings = newGroupingList;
            layoutChanged();
        }
    }

    bool GroupingsOfItemModel::removeRows(int row, int count, const QModelIndex &parent)
    {
        Q_UNUSED(parent)
        if((row + count) > mGroupings.size() || row < 0 || count < 1){
            return false;
        }
        layoutAboutToBeChanged();
        mGroupings.erase(mGroupings.begin() + row, mGroupings.begin() + (row + count));
        layoutChanged();
        return true;
    }

    // ================================================
    //  Event Handler
    // ================================================



    void GroupingsOfItemModel::handleGroupingRemoved(Grouping* grp)
    {
        int idx = getIndexOfGrouping(grp);
        if(idx < 0)
            return;
        removeRow(idx);
    }

    void GroupingsOfItemModel::handleGroupingNameChanged(Grouping* grp)
    {
        int idx = getIndexOfGrouping(grp);
        if(idx < 0)
            return;
        mGroupings[idx].setName(QString::fromStdString(grp->get_name()));
        dataChanged(index(idx,0),index(idx,0));
    }

    // These 'assigned' handler functions are temporary since there is no support for multiple groupings per item (yet).

    void GroupingsOfItemModel::handleGroupingGateAssigned(Grouping* grp, u32 id)
    {
        Q_UNUSED(grp)
        if(mItemType == ItemType::Gate && mItemId == id)
        {
            setGate(gNetlist->get_gate_by_id(id));
        }
    }

    void GroupingsOfItemModel::handleGroupingGateRemoved(Grouping* grp, u32 id)
    {
        Q_UNUSED(grp)
        if(mItemType == ItemType::Gate && mItemId == id)
        {
            int idx = getIndexOfGrouping(grp);
            if(idx < 0)
                return;
            removeRow(idx);
        }
    }

    void GroupingsOfItemModel::handleGroupingNetAssigned(Grouping* grp, u32 id)
    {
        Q_UNUSED(grp);
        if(mItemType == ItemType::Net && mItemId == id)
        {
            setNet(gNetlist->get_net_by_id(id));
        }
    }

    void GroupingsOfItemModel::handleGroupingNetRemoved(Grouping* grp, u32 id)
    {
        if(mItemType == ItemType::Net && mItemId == id)
        {
            int idx = getIndexOfGrouping(grp);
            if(idx < 0)
                return;
            removeRow(idx);
        }
    }

    void GroupingsOfItemModel::handleGroupingModuleAssigned(Grouping* grp, u32 id)
    {
        Q_UNUSED(grp)
        if(mItemType == ItemType::Module && mItemId == id)
        {
            setModule(gNetlist->get_module_by_id(id));
        }
    }

    void GroupingsOfItemModel::handleGroupingModuleRemoved(Grouping* grp, u32 id)
    {
        if(mItemType == ItemType::Module && mItemId == id)
        {
            int idx = getIndexOfGrouping(grp);
            if(idx < 0)
                return;
            removeRow(idx);
        }
    }

    void GroupingsOfItemModel::handleGroupingColorChanged(Grouping* grp)
    {
        int idx = getIndexOfGrouping(grp);
        if(idx < 0)
            return;
        mGroupings[idx].setColor(gContentManager->getGroupingManagerWidget()->getModel()->colorForGrouping(grp));
        dataChanged(index(idx,2),index(idx,2));
    }


    int GroupingsOfItemModel::getIndexOfGrouping(Grouping* grp) const
    {
        auto res = std::find_if(mGroupings.begin(), mGroupings.end(), 
            [grp](GroupingTableEntry grpEntry) 
            { 
                return grpEntry.grouping() == grp; 
            });
        if(res != mGroupings.end()){
            return (res-mGroupings.begin());
        }
        else{
            return -1;
        }
    }


} // namespace hal
