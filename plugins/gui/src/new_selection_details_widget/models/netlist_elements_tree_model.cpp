#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{
    NetlistElementsTreeModel::NetlistElementsTreeModel(QObject *parent) : BaseTreeModel(parent), mModuleIcon(QIcon(":/icons/sel_module")), mGateIcon(QIcon(":/icons/sel_gate")), mNetIcon(QIcon(":/icons/sel_net"))
    {
        // use root item to store header information
        //mRootItem = new TreeItem(QList<QVariant>() << "Name" << "ID" << "Type");
        setHeaderLabels(QList<QVariant>() << "Name" << "ID" << "Type");
        setModule(gNetlist->get_module_by_id(1), true, true, true);
        //setContent(QList<int>() << 1, QList<int>(), QList<int>());

        // CONNECTIONS
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &NetlistElementsTreeModel::gateNameChanged);
        connect(gNetlistRelay, &NetlistRelay::gateRemoved, this, &NetlistElementsTreeModel::gateRemoved);
        connect(gNetlistRelay, &NetlistRelay::netRemoved, this, &NetlistElementsTreeModel::netRemoved);

    }

    NetlistElementsTreeModel::~NetlistElementsTreeModel()
    {
        delete mRootItem;
    }

    QVariant NetlistElementsTreeModel::data(const QModelIndex &index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        TreeItem* item = getItemFromIndex(index);
        if(!item)
            return QVariant();

        if(role == Qt::DecorationRole && index.column() == 0)
            return getIconFromItem(getItemFromIndex(index));

        //yes, it performs the same two checks again, should be okay though (in terms of performance)
        return BaseTreeModel::data(index, role);
    }

    bool NetlistElementsTreeModel::removeRows(int row, int count, const QModelIndex &parent)
    {
        Q_UNUSED(count)
        Q_UNUSED(row)
        Q_UNUSED(parent)
        return false;
//        TreeItem* parentItem = getItemFromIndex(parent);
//        TreeItem* itemToRemove = parentItem->getChild(row);
//        int type = itemToRemove->getAdditionalData(mItemTypeKey).toUInt();

//        if(type == itemType::net || type == itemType::gate)
//        {
//            beginRemoveRows(parent, row, row);
//            parentItem->removeChild(itemToRemove);
//            //must be tested, use mDataToBeRemoved as a means to communicate between functions without parameter
//            if(type == itemType::net)
//                mNetToTreeitems.remove((Net*)mDataToBeRemoved, itemToRemove);
//            else
//                mGateToTreeitems.remove((Gate*)mDataToBeRemoved, itemToRemove);
//            endRemoveRows();
//            delete itemToRemove;
//            return true;
//        }
//        else //module type
//        {
//            beginResetModel();
//            TreeItem* removedItem = getItemFromIndex(parent)->removeChildAtPos(row);
//            //DFS /BFS to store all child items of the module and then remove them from the corresponding maps
//            //(must be done before deleteing the removedItem item.
//            QList<TreeItem*> itemsToRemoveFromMaps;
//            QQueue<TreeItem*> queue;
//            queue.enqueue(removedItem);
//            while(!queue.isEmpty())
//            {
//                TreeItem* currentItem = queue.dequeue();
//                itemsToRemoveFromMaps.append(currentItem);
//                for(TreeItem* childItem : currentItem->getChildren())
//                    queue.enqueue(childItem);
//            }

//            for(TreeItem* item : itemsToRemoveFromMaps)
//            {
////                switch (item->getAdditionalData(mItemTypeKey).toInt())
////                {
////                    case itemType::gate: mGateToTreeitems
////                case itemType::net:
////                case itemType::module:

////                }
//            }

//            endResetModel();

//        }

//        return true;
    }

    void NetlistElementsTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleToTreeitems.clear();
        mGateToTreeitems.clear();
        mNetToTreeitems.clear();
    }

    void NetlistElementsTreeModel::setContent(QList<int> modIds, QList<int> gateIds, QList<int> netIds, bool displayModulesRecursive, bool showGatesInSubmods, bool showNetsInSubmods)
    {
        clear();
        beginResetModel();
        for(int id : modIds)
        {
            Module* mod = gNetlist->get_module_by_id(id);
            if(!mod)
                continue;
            TreeItem* modItem = new TreeItem(QList<QVariant>() << QString::fromStdString(mod->get_name())
                                                << mod->get_id() << QString::fromStdString(mod->get_type()));
            if(displayModulesRecursive)
                moduleRecursive(mod, modItem, showGatesInSubmods, showNetsInSubmods);
            modItem->setAdditionalData(mItemTypeKey, itemType::module);
            mRootItem->appendChild(modItem);
            mModuleToTreeitems.insert(mod, modItem);
        }
        //no need to check if gates should be displayed, because if not, just give a empty gateIds list (same for nets)
        for(int id : gateIds)
        {
            Gate* gate = gNetlist->get_gate_by_id(id);
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                              << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
            gateItem->setAdditionalData(mItemTypeKey, itemType::gate);
            mRootItem->appendChild(gateItem);
            mGateToTreeitems.insert(gate, gateItem);
        }
        for(int id : netIds)
        {
            Net* net = gNetlist->get_net_by_id(id);
            TreeItem* netItem = new TreeItem(QList<QVariant>() << QString::fromStdString(net->get_name())
                                             << net->get_id() << "");
            netItem->setAdditionalData(mItemTypeKey, itemType::net);
            mRootItem->appendChild(netItem);
            mNetToTreeitems.insert(net, netItem);
        }
        endResetModel();
    }

    void NetlistElementsTreeModel::setModule(Module* mod, bool showGates, bool showNets, bool displayModulesRecursive)
    {
        QList<int> subModIds, gateIds, netIds;
        for(auto subMod : mod->get_submodules())
            subModIds.append(subMod->get_id());

        if(showGates)
            for(auto gate : mod->get_gates())
                gateIds.append(gate->get_id());

        if(showNets)
            for(auto net : mod->get_internal_nets())
                netIds.append(net->get_id());

        setContent(subModIds, gateIds, netIds, displayModulesRecursive, showGates, showNets);
    }

    void NetlistElementsTreeModel::gateNameChanged(Gate *g)
    {
        for(TreeItem* gateItem : mGateToTreeitems.values(g))
        {
            gateItem->setDataAtIndex(sNameColumn, QString::fromStdString(g->get_name()));
            QModelIndex inx0 = getIndexFromItem(gateItem);
            QModelIndex inx1 = createIndex(inx0.row(), sNameColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void NetlistElementsTreeModel::gateRemoved(Gate *g)
    {
        QList<TreeItem*> items = mGateToTreeitems.values(g);
        for(TreeItem* gateItem : items)
        {
            beginRemoveRows(parent(getIndexFromItem(gateItem)), gateItem->getOwnRow(), gateItem->getOwnRow());
            gateItem->getParent()->removeChild(gateItem);
            endRemoveRows();
            mGateToTreeitems.remove(g, gateItem);
            //a whole lot more complex for modules, because deleting a module item deletes all children, these should be first
            //removed from the maps (i think), and: what is with beginRemoveRows??->simply calling beginResetModel might be the easiest solution..
            delete gateItem;
        }
    }

    void NetlistElementsTreeModel::netNameChanged(Net *n)
    {
        for(TreeItem* netItem : mNetToTreeitems.values(n))
        {
            netItem->setDataAtIndex(sNameColumn, QString::fromStdString(n->get_name()));
            QModelIndex inx0 = getIndexFromItem(netItem);
            QModelIndex inx1 = createIndex(inx0.row(), sNameColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void NetlistElementsTreeModel::netRemoved(Net *n)
    {
        QList<TreeItem*> items = mNetToTreeitems.values(n);
        for(TreeItem* netItem : items)
        {
            beginRemoveRows(parent(getIndexFromItem(netItem)), netItem->getOwnRow(), netItem->getOwnRow());
            netItem->getParent()->removeChild(netItem);
            endRemoveRows();
            mNetToTreeitems.remove(n, netItem);
            //a whole lot more complex for modules, because deleting a module item deletes all children, these should be first
            //removed from the maps (i think), and: what is with beginRemoveRows??->simply calling beginResetModel might be the easiest solution..
            delete netItem;
        }
    }

    void NetlistElementsTreeModel::moduleRecursive(Module* mod, TreeItem* modItem, bool showGates, bool showNets)
    {
        TreeItem* subModItem = nullptr;
        for(Module* subMod : mod->get_submodules())
        {
            subModItem = new TreeItem(QList<QVariant>() << QString::fromStdString(subMod->get_name())
                                                << subMod->get_id() << QString::fromStdString(subMod->get_type()));
            moduleRecursive(subMod, subModItem, showGates);
            subModItem->setAdditionalData(mItemTypeKey, itemType::module);
            modItem->appendChild(subModItem);
            mModuleToTreeitems.insert(subMod, subModItem);
        }
        if(showGates)
        {
            for(auto gate : mod->get_gates())
            {
                TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                                  << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
                gateItem->setAdditionalData(mItemTypeKey, itemType::gate);
                modItem->appendChild(gateItem);
                mGateToTreeitems.insert(gate, gateItem);
            }
        }
        if(showNets)
        {
            for(auto net : mod->get_internal_nets())
            {
                TreeItem* netItem = new TreeItem(QList<QVariant>() << QString::fromStdString(net->get_name())
                                                 << net->get_id() << "");
                netItem->setAdditionalData(mItemTypeKey, itemType::net);
                modItem->appendChild(netItem);
                mNetToTreeitems.insert(net, netItem);
            }
        }
    }

    QIcon NetlistElementsTreeModel::getIconFromItem(TreeItem *item) const
    {
        if(!item)
            return mGateIcon;//some cool gate icon if the item is a nullptr

        switch (item->getAdditionalData(mItemTypeKey).toUInt())
        {
        case itemType::module: return mModuleIcon;
        case itemType::gate: return mGateIcon;
        case itemType::net: return mNetIcon;
        default: return mGateIcon;
        }
    }

}
