#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QDebug>
#include <QQueue>

namespace hal
{
    NetlistElementsTreeModel::NetlistElementsTreeModel(QObject *parent) : BaseTreeModel(parent), mModuleIcon(QIcon(":/icons/sel_module")), mGateIcon(QIcon(":/icons/sel_gate")), mNetIcon(QIcon(":/icons/sel_net")),
         mGatesDisplayed(true), mNetsDisplayed(true), mDisplaySubmodRecursive(true), mCurrentlyDisplayingModule(false), mModId(-1)
    {
        // use root item to store header information
        //mRootItem = new TreeItem(QList<QVariant>() << "Name" << "ID" << "Type");
        setHeaderLabels(QList<QVariant>() << "Name" << "ID" << "Type");
        //setModule(gNetlist->get_module_by_id(1), true, true, true);
        //setContent(QList<int>() << 1, QList<int>(), QList<int>());

        // CONNECTIONS
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &NetlistElementsTreeModel::gateNameChanged);
        connect(gNetlistRelay, &NetlistRelay::gateRemoved, this, &NetlistElementsTreeModel::gateRemoved);
        connect(gNetlistRelay, &NetlistRelay::netRemoved, this, &NetlistElementsTreeModel::netRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &NetlistElementsTreeModel::moduleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleTypeChanged, this, &NetlistElementsTreeModel::moduleTypeChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &NetlistElementsTreeModel::moduleGateRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &NetlistElementsTreeModel::moduleGateAssigned);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &NetlistElementsTreeModel::moduleSubmoduleRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleAdded, this, &NetlistElementsTreeModel::moduleSubmoduleAdded);
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

    void NetlistElementsTreeModel::clear()
    {
        BaseTreeModel::clear();
        mModuleToTreeitems.clear();
        mGateToTreeitems.clear();
        mNetToTreeitems.clear();
        mDisplaySubmodRecursive = true;
        mGatesDisplayed = true;
        mNetsDisplayed = true;
        mCurrentlyDisplayingModule = false;
        mModId = -1;
    }

    void NetlistElementsTreeModel::setContent(QList<int> modIds, QList<int> gateIds, QList<int> netIds, bool displayModulesRecursive, bool showGatesInSubmods, bool showNetsInSubmods)
    {
        mDisplaySubmodRecursive = displayModulesRecursive;
        mGatesDisplayed = showGatesInSubmods;
        mNetsDisplayed = showNetsInSubmods;

        //i need to temp. store this because clear() is called....
        bool disPlayedModtmp = mCurrentlyDisplayingModule;
        int modIdtmp = mModId;

        clear();

        mCurrentlyDisplayingModule = disPlayedModtmp;
        mModId = modIdtmp;

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
            //modItem->setAdditionalData(keyItemType, itemType::module);
            modItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            modItem->setAdditionalData(keyRepresentedID, mod->get_id());
            mRootItem->appendChild(modItem);
            mModuleToTreeitems.insert(mod, modItem);
        }
        //no need to check if gates should be displayed, because if not, just give a empty gateIds list (same for nets)
        for(int id : gateIds)
        {
            Gate* gate = gNetlist->get_gate_by_id(id);
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                              << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
            //gateItem->setAdditionalData(keyItemType, itemType::gate);
            gateItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(keyRepresentedID, gate->get_id());
            mRootItem->appendChild(gateItem);
            mGateToTreeitems.insert(gate, gateItem);
        }
        for(int id : netIds)
        {
            Net* net = gNetlist->get_net_by_id(id);
            TreeItem* netItem = new TreeItem(QList<QVariant>() << QString::fromStdString(net->get_name())
                                             << net->get_id() << "");
            //netItem->setAdditionalData(keyItemType, itemType::net);
            netItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::net));
            netItem->setAdditionalData(keyRepresentedID, net->get_id());
            mRootItem->appendChild(netItem);
            mNetToTreeitems.insert(net, netItem);
        }
        endResetModel();
    }

    void NetlistElementsTreeModel::setModule(Module* mod, bool showGates, bool showNets, bool displayModulesRecursive)
    {
        mCurrentlyDisplayingModule = true;
        mModId = mod->get_id();

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

    NetlistElementsTreeModel::itemType NetlistElementsTreeModel::getTypeOfItem(TreeItem *item) const
    {
        return item->getAdditionalData(keyItemType).value<itemType>();
    }

    int NetlistElementsTreeModel::getRepresentedIdOfItem(TreeItem *item) const
    {
        return item->getAdditionalData(keyRepresentedID).toInt();
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
            if(mNetsDisplayed && ( (gateItem->getParent() == mRootItem && mCurrentlyDisplayingModule) || gateItem->getParent() != mRootItem))
            {
                beginResetModel();
                updateInternalNetsOfModule(gateItem->getParent());//perhaps for all parents? go until the mRootItem node?
                endResetModel();
            }
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
            delete netItem;
        }
    }

    void NetlistElementsTreeModel::moduleNameChanged(Module *m)
    {
        for(TreeItem* modItem : mModuleToTreeitems.values(m))
        {
            modItem->setDataAtIndex(sNameColumn, QString::fromStdString(m->get_name()));
            QModelIndex inx0 = getIndexFromItem(modItem);
            Q_EMIT dataChanged(inx0, createIndex(inx0.row(), sNameColumn, inx0.internalPointer()));
        }
    }

    void NetlistElementsTreeModel::moduleTypeChanged(Module *m)
    {
        for(TreeItem* modItem : mModuleToTreeitems.values(m))
        {
            modItem->setDataAtIndex(sTypeColumn, QString::fromStdString(m->get_type()));
            QModelIndex inx0 = getIndexFromItem(modItem);
            Q_EMIT dataChanged(inx0, createIndex(inx0.row(), sTypeColumn, inx0.internalPointer()));
        }
    }

    void NetlistElementsTreeModel::moduleSubmoduleRemoved(Module *m, int removed_module)
    {
        Q_UNUSED(m)
        //1. go through the actual TreeItems through a BFS and remove them from the maps
        //2. delete the associated module tree items (beginResetModel)
        Module* removedMod = gNetlist->get_module_by_id(removed_module);
        QList<TreeItem*> tmpSubmodItems; // they already get removed in the BFS
        for(TreeItem* removedSubmodItem : mModuleToTreeitems.values(removedMod))
        {
            tmpSubmodItems.append(removedSubmodItem);
            QQueue<TreeItem*> treeItemsQueue;
            treeItemsQueue.enqueue(removedSubmodItem);
            while(!treeItemsQueue.isEmpty())
            {
                TreeItem* currentItem = treeItemsQueue.dequeue();
                int id = currentItem->getAdditionalData(keyRepresentedID).toUInt();

                switch (getTypeOfItem(currentItem))
                {
                case itemType::module: mModuleToTreeitems.remove(gNetlist->get_module_by_id(id),currentItem); break;
                case itemType::gate: mGateToTreeitems.remove(gNetlist->get_gate_by_id(id), currentItem); break;
                case itemType::net: mNetToTreeitems.remove(gNetlist->get_net_by_id(id), currentItem); break;
                }

                for(TreeItem* child : currentItem->getChildren())
                    treeItemsQueue.enqueue(child);
            }
        }
        //after clearing the maps, delete the corresponding module items (propagates through all children)
        beginResetModel();
        for(TreeItem* removedSubItem : tmpSubmodItems)
        {
            removedSubItem->getParent()->removeChild(removedSubItem);
            if(mNetsDisplayed)
                updateInternalNetsOfModule(removedSubItem->getParent());
            delete removedSubItem;
        }
        endResetModel();
    }

    void NetlistElementsTreeModel::moduleGateAssigned(Module *m, int assigned_gate)
    {
        //TODO: tidy up, since both cases (special and not special) are handled the same, perhaps
        //put the code in an external function [ insertGateIntoMod(treeItem, gate) ]
        if(!mGatesDisplayed)
            return;

        //special case when we actually displaying the content of a module through setModule
        Gate* assignedGate = gNetlist->get_gate_by_id(assigned_gate);
        if(mCurrentlyDisplayingModule && mModId == (int)m->get_id())
        {
            //insert gate at the first index possible(nets must also be invalidated?)
            int indexToInsert = 0;
            for(; indexToInsert < mRootItem->getChildCount(); indexToInsert++)
            {
                if(getTypeOfItem(mRootItem->getChild(indexToInsert)) != itemType::module)
                    break;
            }
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(assignedGate->get_name())
                                              << assignedGate->get_id() << QString::fromStdString(assignedGate->get_type()->get_name()));
            //gateItem->setAdditionalData(keyItemType, itemType::gate);
            gateItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(keyRepresentedID, assignedGate->get_id());
            beginResetModel();//only do that if "all" nets are invalidated that this module holds
            mRootItem->insertChild(indexToInsert, gateItem);
            mGateToTreeitems.insert(assignedGate, gateItem);
            if(mNetsDisplayed)
                updateInternalNetsOfModule(mRootItem);
            endResetModel();
            return;
        }
        //standard case in which you do the same as obove, but just go through each module item
        for(TreeItem* modItem : mModuleToTreeitems.values(m))
        {
            int indexToInsert = 0;
            for(; indexToInsert < modItem->getChildCount(); indexToInsert++)
                if(getTypeOfItem(modItem->getChild(indexToInsert)) != itemType::module)
                    break;
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(assignedGate->get_name())
                                              << assignedGate->get_id() << QString::fromStdString(assignedGate->get_type()->get_name()));
            //gateItem->setAdditionalData(keyItemType, itemType::gate);
            gateItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(keyRepresentedID, assignedGate->get_id());
            //beginInsertRows(getIndexFromItem(modItem), indexToInsert, indexToInsert);
            beginResetModel();
            modItem->insertChild(indexToInsert, gateItem);
            //endInsertRows();
            mGateToTreeitems.insert(assignedGate, gateItem);
            if(mNetsDisplayed)
                updateInternalNetsOfModule(modItem);
            endResetModel();
            return;
        }
    }

    void NetlistElementsTreeModel::moduleGateRemoved(Module *m, int removed_gate)
    {
        Q_UNUSED(m) //does not depend on the module but on the gate, simply removed them..
        gateRemoved(gNetlist->get_gate_by_id(removed_gate));
    }

    void NetlistElementsTreeModel::moduleSubmoduleAdded(Module *m, int added_module)
    {
        beginResetModel();
        Module* addedModule = gNetlist->get_module_by_id(added_module);

        //special case when a module is represented with setModule
        if(mCurrentlyDisplayingModule && (int)m->get_id() == mModId)
        {
            TreeItem* addedSubmodItem = new TreeItem(QList<QVariant>() << QString::fromStdString(addedModule->get_name()) << addedModule->get_id()
                                                     << QString::fromStdString(addedModule->get_type()));
            moduleRecursive(addedModule, addedSubmodItem, mGatesDisplayed, mNetsDisplayed);
            //addedSubmodItem->setAdditionalData(keyItemType, itemType::module);
            addedSubmodItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            addedSubmodItem->setAdditionalData(keyRepresentedID, addedModule->get_id());
            mRootItem->insertChild(0, addedSubmodItem);
            mModuleToTreeitems.insert(addedModule, addedSubmodItem);
            if(mNetsDisplayed)
                updateInternalNetsOfModule(mRootItem);
            return;
        }

        //standard case for all displayed things
        for(TreeItem* parentModItem : mModuleToTreeitems.values(m))
        {
            TreeItem* addedSubmodItem = new TreeItem(QList<QVariant>() << QString::fromStdString(addedModule->get_name()) << addedModule->get_id()
                                                     << QString::fromStdString(addedModule->get_type()));
            moduleRecursive(addedModule, addedSubmodItem, mGatesDisplayed, mNetsDisplayed);
            //addedSubmodItem->setAdditionalData(keyItemType, itemType::module);
            addedSubmodItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            addedSubmodItem->setAdditionalData(keyRepresentedID, addedModule->get_id());
            parentModItem->insertChild(0, addedSubmodItem);
            mModuleToTreeitems.insert(addedModule, addedSubmodItem);
            if(mNetsDisplayed)
                updateInternalNetsOfModule(parentModItem);
        }
        endResetModel();
    }

    void NetlistElementsTreeModel::moduleRecursive(Module* mod, TreeItem* modItem, bool showGates, bool showNets)
    {
        TreeItem* subModItem = nullptr;
        for(Module* subMod : mod->get_submodules())
        {
            subModItem = new TreeItem(QList<QVariant>() << QString::fromStdString(subMod->get_name())
                                                << subMod->get_id() << QString::fromStdString(subMod->get_type()));
            moduleRecursive(subMod, subModItem, showGates);
            //subModItem->setAdditionalData(keyItemType, itemType::module);
            subModItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            subModItem->setAdditionalData(keyRepresentedID, subMod->get_id());
            modItem->appendChild(subModItem);
            mModuleToTreeitems.insert(subMod, subModItem);
        }
        if(showGates)
        {
            for(auto gate : mod->get_gates())
            {
                TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                                  << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
                //gateItem->setAdditionalData(keyItemType, itemType::gate);
                gateItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::gate));
                gateItem->setAdditionalData(keyRepresentedID, gate->get_id());
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
                //netItem->setAdditionalData(keyItemType, itemType::net);
                netItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::net));
                netItem->setAdditionalData(keyRepresentedID, net->get_id());
                modItem->appendChild(netItem);
                mNetToTreeitems.insert(net, netItem);
            }
        }
    }

    QIcon NetlistElementsTreeModel::getIconFromItem(TreeItem *item) const
    {
        if(!item)
            return mGateIcon;//some cool gate icon if the item is a nullptr

        switch (getTypeOfItem(item))
        {
        case itemType::module: return mModuleIcon;
        case itemType::gate: return mGateIcon;
        case itemType::net: return mNetIcon;
        default: return mGateIcon;
        }
    }

    void NetlistElementsTreeModel::updateInternalNetsOfModule(TreeItem *moduleItem)
    {
        int moduleId = (moduleItem == mRootItem) ? mModId : moduleItem->getAdditionalData(keyRepresentedID).toInt();
        Module* mod = gNetlist->get_module_by_id(moduleId);
        //remove and delte the last child of the module-item until no net items are left
        while(moduleItem->getChildCount() > 0 && getTypeOfItem(moduleItem->getChild(moduleItem->getChildCount()-1)) == itemType::net)
        {
            TreeItem* lastNetItem = moduleItem->removeChildAtPos(moduleItem->getChildCount()-1);
            mNetToTreeitems.remove(gNetlist->get_net_by_id(lastNetItem->getAdditionalData(keyRepresentedID).toUInt()), lastNetItem);
            delete lastNetItem;
        }
        //append (potentionally) new internal nets
        for(Net* n : mod->get_internal_nets())
        {
            TreeItem* netItem = new TreeItem(QList<QVariant>() << QString::fromStdString(n->get_name()) << n->get_id() << "");
            //netItem->setAdditionalData(keyItemType, itemType::net);
            netItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::net));
            netItem->setAdditionalData(keyRepresentedID, n->get_id());
            mNetToTreeitems.insert(n, netItem);
            moduleItem->appendChild(netItem);
        }
    }

}
