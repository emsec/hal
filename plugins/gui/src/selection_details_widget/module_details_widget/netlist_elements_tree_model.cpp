#include "gui/selection_details_widget/module_details_widget/netlist_elements_tree_model.h"
#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/basic_tree_model/base_tree_item.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QDebug>
#include <QQueue>

namespace hal
{

    NetlistElementsTreeitem::NetlistElementsTreeitem(const QString &name, int id, QString tp)
        : mType(tp), mId(id), mName(name)
    {;}

    QVariant NetlistElementsTreeitem::getData(int index) const
    {
        switch (index)
        {
        case 0: {
            QVariant qvName = QVariant(mName);
            return qvName;
            break;}
        case 1: {
            QVariant qvId  = QVariant(mId);
            return qvId;
            break;}
        case 2: {
            QVariant qvType = QVariant(mType);
            return qvType;
            break;}
        }
        return QVariant();
    }

    void NetlistElementsTreeitem::setData(QList<QVariant> data)
    {
        mName = data[0].toString();
        mId = data[1].toInt();
        mType = data[2].toString();

    }

    void NetlistElementsTreeitem::setDataAtIndex(int index, QVariant &data)
    {
        const char* ctyp[] = { "module", "gate", "net"};

        switch (index)
        {
        case 0: mName = data.toString(); break;
        case 1: mId   = data.toInt(); break;
        case 2:
            for (int j=0; j<3; j++)
                if (data.toString() == ctyp[j])
                {
                    mType = data.toString();
                    break;
                }
        }
    }

    void NetlistElementsTreeitem::appendData(QVariant data) {}

    int NetlistElementsTreeitem::getColumnCount() const
    {
        return 3;
    }

    NetlistElementsTreeModel::NetlistElementsTreeModel(QObject *parent)
        : BaseTreeModel(parent),
         mGatesDisplayed(true), mNetsDisplayed(true), mDisplaySubmodRecursive(true), mCurrentlyDisplayingModule(false), mModId(-1)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "ID" << "Type");

        // CONNECTIONS
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &NetlistElementsTreeModel::gateNameChanged);
        connect(gNetlistRelay, &NetlistRelay::gateRemoved, this, &NetlistElementsTreeModel::gateRemoved);
        connect(gNetlistRelay, &NetlistRelay::netRemoved, this, &NetlistElementsTreeModel::netRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &NetlistElementsTreeModel::moduleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleTypeChanged, this, &NetlistElementsTreeModel::moduleTypeChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &NetlistElementsTreeModel::moduleGateRemoved, Qt::QueuedConnection);
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

        BaseTreeItem* item = getItemFromIndex(index);
        if(!item)
            return QVariant();

        if(role == Qt::DecorationRole && index.column() == 0) {
            NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(getItemFromIndex(index));
            return getIconFromItem(neti);
        }

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
            NetlistElementsTreeitem* modItem = new NetlistElementsTreeitem(QString::fromStdString(mod->get_name()),
                                                                           mod->get_id(), QString::fromStdString(mod->get_type()));
            if(displayModulesRecursive)
                moduleRecursive(mod, modItem, showGatesInSubmods, showNetsInSubmods);
            modItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            modItem->setAdditionalData(keyRepresentedID, mod->get_id());
            mRootItem->appendChild(modItem);
            mModuleToTreeitems.insert(mod, modItem);
        }
        //no need to check if gates should be displayed, because if not, just give a empty gateIds list (same for nets)
        for(int id : gateIds)
        {
            Gate* gate = gNetlist->get_gate_by_id(id);
            NetlistElementsTreeitem* gateItem = new NetlistElementsTreeitem(QString::fromStdString(gate->get_name()),
                                              gate->get_id(), QString::fromStdString(gate->get_type()->get_name()));
            gateItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(keyRepresentedID, gate->get_id());
            mRootItem->appendChild(gateItem);
            mGateToTreeitems.insert(gate, gateItem);
        }
        for(int id : netIds)
        {
            Net* net = gNetlist->get_net_by_id(id);
            NetlistElementsTreeitem* netItem = new NetlistElementsTreeitem(QString::fromStdString(net->get_name()),
                                             net->get_id(), "");
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
        Q_EMIT numberOfSubmodulesChanged(mod->get_submodules().size());
    }

    NetlistElementsTreeModel::itemType NetlistElementsTreeModel::getTypeOfItem(NetlistElementsTreeitem *item) const
    {
        return item->getAdditionalData(keyItemType).value<itemType>();
    }

    int NetlistElementsTreeModel::getRepresentedIdOfItem(NetlistElementsTreeitem *item) const
    {
        return item->getAdditionalData(keyRepresentedID).toInt();
    }

    void NetlistElementsTreeModel::gateNameChanged(Gate *g)
    {
        for(BaseTreeItem* gateItem : mGateToTreeitems.values(g))
        {
            QVariant data = QVariant(QString::fromStdString(g->get_name()));
            gateItem->setDataAtIndex(sNameColumn, data);
            QModelIndex inx0 = getIndexFromItem(gateItem);
            QModelIndex inx1 = createIndex(inx0.row(), sNameColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void NetlistElementsTreeModel::gateRemoved(Gate *g)
    {
        QList<NetlistElementsTreeitem*> items = mGateToTreeitems.values(g);
        for(NetlistElementsTreeitem* gateItem : items)
        {
            beginRemoveRows(parent(getIndexFromItem(gateItem)), gateItem->getOwnRow(), gateItem->getOwnRow());
            gateItem->getParent()->removeChild(gateItem);
            endRemoveRows();
            mGateToTreeitems.remove(g, gateItem);
            if(mNetsDisplayed && ( (gateItem->getParent() == mRootItem && mCurrentlyDisplayingModule) || gateItem->getParent() != mRootItem))
            {
                beginResetModel();
                NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(gateItem->getParent());
                updateInternalNetsOfModule(neti);//perhaps for all parents? go until the mRootItem node?
                endResetModel();
            }
            delete gateItem;
        }
    }

    void NetlistElementsTreeModel::netNameChanged(Net *n)
    {
        for(NetlistElementsTreeitem* netItem : mNetToTreeitems.values(n))
        {
            QVariant data = QVariant(QString::fromStdString(n->get_name()));
            netItem->setDataAtIndex(sNameColumn, data);
            QModelIndex inx0 = getIndexFromItem(netItem);
            QModelIndex inx1 = createIndex(inx0.row(), sNameColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void NetlistElementsTreeModel::netRemoved(Net *n)
    {
        QList<NetlistElementsTreeitem*> items = mNetToTreeitems.values(n);
        for(NetlistElementsTreeitem* netItem : items)
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
        for(NetlistElementsTreeitem* modItem : mModuleToTreeitems.values(m))
        {
            QVariant data = QVariant(QString::fromStdString(m->get_name()));
            modItem->setDataAtIndex(sNameColumn, data);
            QModelIndex inx0 = getIndexFromItem(modItem);
            Q_EMIT dataChanged(inx0, createIndex(inx0.row(), sNameColumn, inx0.internalPointer()));
        }
    }

    void NetlistElementsTreeModel::moduleTypeChanged(Module *m)
    {
        for(NetlistElementsTreeitem* modItem : mModuleToTreeitems.values(m))
        {
            QVariant data = QVariant(QString::fromStdString(m->get_type()));
            modItem->setDataAtIndex(sTypeColumn, data);
            QModelIndex inx0 = getIndexFromItem(modItem);
            Q_EMIT dataChanged(inx0, createIndex(inx0.row(), sTypeColumn, inx0.internalPointer()));
        }
    }

    void NetlistElementsTreeModel::moduleSubmoduleRemoved(Module *m, int removed_module)
    {
        //1. go through the actual TreeItems through a BFS and remove them from the maps
        //2. delete the associated module tree items (beginResetModel)
        Module* removedMod = gNetlist->get_module_by_id(removed_module);
        QList<NetlistElementsTreeitem*> tmpSubmodItems; // they already get removed in the BFS
        for(NetlistElementsTreeitem* removedSubmodItem : mModuleToTreeitems.values(removedMod))
        {
            tmpSubmodItems.append(removedSubmodItem);
            QQueue<NetlistElementsTreeitem*> treeItemsQueue;
            treeItemsQueue.enqueue(removedSubmodItem);
            while(!treeItemsQueue.isEmpty())
            {
                NetlistElementsTreeitem* currentItem = treeItemsQueue.dequeue();
                int id = currentItem->getAdditionalData(keyRepresentedID).toUInt();

                switch (getTypeOfItem(currentItem))
                {
                case itemType::module: mModuleToTreeitems.remove(gNetlist->get_module_by_id(id),currentItem); break;
                case itemType::gate: mGateToTreeitems.remove(gNetlist->get_gate_by_id(id), currentItem); break;
                case itemType::net: mNetToTreeitems.remove(gNetlist->get_net_by_id(id), currentItem); break;
                }

                for(BaseTreeItem* child : currentItem->getChildren()){
                    NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(child);
                    treeItemsQueue.enqueue(neti);
                }
            }
        }
        //after clearing the maps, delete the corresponding module items (propagates through all children)
        beginResetModel();
        for(NetlistElementsTreeitem* removedSubItem : tmpSubmodItems)
        {
            removedSubItem->getParent()->removeChild(removedSubItem);
            if(mNetsDisplayed) {
                NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(removedSubItem->getParent());
                updateInternalNetsOfModule(neti);
            }
            delete removedSubItem;
        }
        endResetModel();

        if((int)m->get_id() == mModId)
            Q_EMIT numberOfSubmodulesChanged(m->get_submodules().size());

        if(removed_module == mModId)
            clear();
    }

    void NetlistElementsTreeModel::moduleGateAssigned(Module *m, int assigned_gate)
    {
        if(!mGatesDisplayed || (mModuleToTreeitems.values(m).isEmpty() && !(mCurrentlyDisplayingModule && mModId != (int)m->get_id())))
            return;

        Gate* assignedGate = gNetlist->get_gate_by_id(assigned_gate);

        //helper lambda function to parametrize the moduleItem parent (not worth own named class function)
        auto appendNewGateToModule = [this, assignedGate](BaseTreeItem* modItem){
            int indexToInsert = 0;
            for(; indexToInsert < modItem->getChildCount(); indexToInsert++) {
                NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(modItem->getChild(indexToInsert));
                if(getTypeOfItem(neti) != itemType::module)
                    break;
            }

            NetlistElementsTreeitem* gateItem = new NetlistElementsTreeitem(QString::fromStdString(assignedGate->get_name()),
                                              assignedGate->get_id(), QString::fromStdString(assignedGate->get_type()->get_name()));
            gateItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(keyRepresentedID, assignedGate->get_id());
            //beginInsertRows(getIndexFromItem(modItem), indexToInsert, indexToInsert);
            beginResetModel();
            modItem->insertChild(indexToInsert, gateItem);
            //endInsertRows();
            mGateToTreeitems.insert(assignedGate, gateItem);
            if(mNetsDisplayed) {
                NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(modItem);
                updateInternalNetsOfModule(neti);
            }
            endResetModel();
        };

        //special case when we actually displaying the content of a module through setModule
        if(mCurrentlyDisplayingModule && mModId == (int)m->get_id())
            appendNewGateToModule(mRootItem);

        //standard case in which you do the same as obove, but just go through each module item
        for(BaseTreeItem* modItem : mModuleToTreeitems.values(m))
            appendNewGateToModule(modItem);
    }

    void NetlistElementsTreeModel::moduleGateRemoved(Module *m, int removed_gate)
    {
        Q_UNUSED(m) //does not depend on the module but on the gate, simply removed them..
        gateRemoved(gNetlist->get_gate_by_id(removed_gate));
    }

    void NetlistElementsTreeModel::moduleSubmoduleAdded(Module *m, int added_module)
    {
        if(mModuleToTreeitems.values(m).isEmpty() && !(mCurrentlyDisplayingModule && (int)m->get_id() == mModId))
            return;

        beginResetModel();
        Module* addedModule = gNetlist->get_module_by_id(added_module);

        auto appendNewSubmodItem = [this, addedModule](BaseTreeItem* parentModItem){
            NetlistElementsTreeitem* addedSubmodItem = new NetlistElementsTreeitem(QString::fromStdString(addedModule->get_name()),
                                                           addedModule->get_id(), QString::fromStdString(addedModule->get_type()));
            moduleRecursive(addedModule, addedSubmodItem, mGatesDisplayed, mNetsDisplayed);
            addedSubmodItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            addedSubmodItem->setAdditionalData(keyRepresentedID, addedModule->get_id());
            parentModItem->insertChild(0, addedSubmodItem);
            mModuleToTreeitems.insert(addedModule, addedSubmodItem);
            if(mNetsDisplayed) {
                NetlistElementsTreeitem* neti = static_cast<NetlistElementsTreeitem*>(parentModItem);
                updateInternalNetsOfModule(neti);
            }
        };

        //special case when a module is represented with setModule
        if(mCurrentlyDisplayingModule && (int)m->get_id() == mModId)
        {
            appendNewSubmodItem(mRootItem);
            Q_EMIT numberOfSubmodulesChanged(m->get_submodules().size());
        }

        //standard case for all displayed things
        for(NetlistElementsTreeitem* parentModItem : mModuleToTreeitems.values(m))
            appendNewSubmodItem(parentModItem);

        endResetModel();
    }

    void NetlistElementsTreeModel::moduleRecursive(Module* mod, NetlistElementsTreeitem* modItem, bool showGates, bool showNets)
    {
        NetlistElementsTreeitem* subModItem = nullptr;
        for(Module* subMod : mod->get_submodules())
        {
            subModItem = new NetlistElementsTreeitem(QString::fromStdString(subMod->get_name()),
                                                subMod->get_id(), QString::fromStdString(subMod->get_type()));
            moduleRecursive(subMod, subModItem, showGates);
            subModItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::module));
            subModItem->setAdditionalData(keyRepresentedID, subMod->get_id());
            modItem->appendChild(subModItem);
            mModuleToTreeitems.insert(subMod, subModItem);
        }
        if(showGates)
        {
            for(auto gate : mod->get_gates())
            {
                NetlistElementsTreeitem* gateItem = new NetlistElementsTreeitem(QString::fromStdString(gate->get_name()),
                                                  gate->get_id(), QString::fromStdString(gate->get_type()->get_name()));
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
                NetlistElementsTreeitem* netItem = new NetlistElementsTreeitem(QString::fromStdString(net->get_name()),
                                                 net->get_id(), "");
                netItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::net));
                netItem->setAdditionalData(keyRepresentedID, net->get_id());
                modItem->appendChild(netItem);
                mNetToTreeitems.insert(net, netItem);
            }
        }
    }

    QIcon NetlistElementsTreeModel::getIconFromItem(NetlistElementsTreeitem *item) const
    {
        if(!item)
            return QIcon();

        u32 id = item->getData(1).toInt();
        switch (getTypeOfItem(item))
        {
        case itemType::module:
            return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ModuleIcon,id));
        case itemType::gate:
            return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::GateIcon,id));
        default:
            return QIcon();
        }
   }

    void NetlistElementsTreeModel::updateInternalNetsOfModule(NetlistElementsTreeitem *moduleItem)
    {
        BaseTreeItem* moduleItemBase = static_cast<BaseTreeItem*>(moduleItem);
       BaseTreeItem* mRootBase = static_cast<BaseTreeItem*>(mRootItem);
        int moduleId = (moduleItemBase == mRootBase) ? mModId : moduleItem->getAdditionalData(keyRepresentedID).toInt();
        Module* mod = gNetlist->get_module_by_id(moduleId);
        //remove and delte the last child of the module-item until no net items are left
        while(moduleItem->getChildCount() > 0 && getTypeOfItem(static_cast<NetlistElementsTreeitem*>(moduleItem->getChild(moduleItem->getChildCount()-1))) == itemType::net)
        {
            NetlistElementsTreeitem* lastNetItem = static_cast<NetlistElementsTreeitem*>(moduleItem->removeChildAtPos(moduleItem->getChildCount()-1));
            mNetToTreeitems.remove(gNetlist->get_net_by_id(lastNetItem->getAdditionalData(keyRepresentedID).toUInt()), lastNetItem);
            delete lastNetItem;
        }
        //append (potentionally) new internal nets
        for(Net* n : mod->get_internal_nets())
        {
            NetlistElementsTreeitem* netItem = new NetlistElementsTreeitem(QString::fromStdString(n->get_name()),n->get_id(), "");
            //netItem->setAdditionalData(keyItemType, itemType::net);
            netItem->setAdditionalData(keyItemType, QVariant::fromValue(itemType::net));
            netItem->setAdditionalData(keyRepresentedID, n->get_id());
            mNetToTreeitems.insert(n, netItem);
            moduleItem->appendChild(netItem);
        }
    }


}
