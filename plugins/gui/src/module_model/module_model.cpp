#include "gui/module_model/module_model.h"

#include "gui/gui_globals.h"

#include "gui/selection_details_widget/selection_details_icon_provider.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    ModuleModel::ModuleModel(QObject* parent) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "ID" << "Type");
        connect(gNetlistRelay, &NetlistRelay::moduleCreated,          this, &ModuleModel::handleModuleCreated);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged,      this, &ModuleModel::handleModuleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleParentChanged,    this, &ModuleModel::handleModuleParentChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleAdded,   this, &ModuleModel::handleModuleSubmoduleAdded);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &ModuleModel::handleModuleSubmoduleRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned,     this, &ModuleModel::handleModuleGateAssigned);
        connect(gNetlistRelay, &NetlistRelay::moduleGatesAssignBegin, this, &ModuleModel::handleModuleGatesAssignBegin);
        connect(gNetlistRelay, &NetlistRelay::moduleGatesAssignEnd,   this, &ModuleModel::handleModuleGatesAssignEnd);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved,      this, &ModuleModel::handleModuleGateRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleRemoved,          this, &ModuleModel::handleModuleRemoved);
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged,        this, &ModuleModel::handleGateNameChanged);
        connect(gNetlistRelay, &NetlistRelay::netCreated,             this, &ModuleModel::handleNetCreated);
        connect(gNetlistRelay, &NetlistRelay::netRemoved,             this, &ModuleModel::handleNetRemoved);
        connect(gNetlistRelay, &NetlistRelay::netNameChanged,         this, &ModuleModel::handleNetNameChanged);
        connect(gNetlistRelay, &NetlistRelay::netSourceAdded,         this, &ModuleModel::handleNetUpdated);
        connect(gNetlistRelay, &NetlistRelay::netSourceRemoved,       this, &ModuleModel::handleNetUpdated);
        connect(gNetlistRelay, &NetlistRelay::netDestinationAdded,    this, &ModuleModel::handleNetUpdated);
        connect(gNetlistRelay, &NetlistRelay::netDestinationRemoved,  this, &ModuleModel::handleNetUpdated);
    }

    QVariant ModuleModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        ModuleItem* item = static_cast<ModuleItem*>(index.internalPointer());

        if (!item)
            return QVariant();

        switch (role)
        {
            case Qt::DecorationRole:
            {
                if (index.column() == 0)
                {
                    switch(item->getType()){
                        case ModuleItem::TreeItemType::Module:
                            return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ModuleIcon, item->id()));
                        case ModuleItem::TreeItemType::Gate:
                            return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::GateIcon, item->id()));
                        case ModuleItem::TreeItemType::Net:
                            return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::NetIcon, item->id()));
                    }
                }
                break;
            }
            case Qt::DisplayRole:
            {
                return item->getData(index.column());
            }
            case Qt::ForegroundRole:
            {
                if (item->highlighted())
                    return QColor(QColor(255, 221, 0));    // USE STYLESHEETS
                else
                    return QColor(QColor(255, 255, 255));    // USE STYLESHEETS
            }
            default:
                return QVariant();
        }
        return QVariant();
    }

    Qt::ItemFlags ModuleModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index);
    }

    ModuleItem* ModuleModel::getItem(const QModelIndex& index) const
    {
        if (index.isValid())
            return static_cast<ModuleItem*>(index.internalPointer());
        else
            return nullptr;
    }

    void ModuleModel::init()
    {
        addRecursively(gNetlist->get_top_module());
        moduleAssignNets();
    }

    void ModuleModel::clear()
    {
        beginResetModel();

        BaseTreeModel::clear();
        mModuleMap.clear();
        mGateMap.clear();
        mNetMap.clear();
        endResetModel();
    }

    void ModuleModel::addModule(u32 id, u32 parentId)
    {
        Q_ASSERT(gNetlist->get_module_by_id(id));
        Q_ASSERT(gNetlist->get_module_by_id(parentId));

        for (auto it = mModuleMap.lowerBound(parentId); it != mModuleMap.upperBound(parentId); ++it)
        {
            ModuleItem* parentItem = mModuleMap.value(parentId);
            createChildItem(id, ModuleItem::TreeItemType::Module, parentItem);
        }
    }

    void ModuleModel::addGate(u32 id, u32 parentId)
    {
        Q_ASSERT(gNetlist->get_gate_by_id(id));
        Q_ASSERT(gNetlist->get_module_by_id(parentId));


        for (auto it = mModuleMap.lowerBound(parentId); it != mModuleMap.upperBound(parentId); ++it)
        {
            createChildItem(id, ModuleItem::TreeItemType::Gate, it.value());
        }
    }

    void ModuleModel::addNet(u32 id, u32 parentId)
    {
        Q_ASSERT(gNetlist->get_net_by_id(id));
        Q_ASSERT(gNetlist->get_module_by_id(parentId));

        for (auto it = mModuleMap.lowerBound(parentId); it != mModuleMap.upperBound(parentId); ++it)
        {
            createChildItem(id, ModuleItem::TreeItemType::Net, it.value());
        }
    }

    void ModuleModel::addRecursively(const Module* module, BaseTreeItem *parentItem)
    {
        Q_ASSERT(module);
        ModuleItem* moduleItem = createChildItem(module->get_id(), ModuleItem::TreeItemType::Module, parentItem ? parentItem : mRootItem);
        Q_ASSERT(moduleItem);
        for(const Module* subModule : module->get_submodules())
            addRecursively(subModule, moduleItem);

        for(const Gate* g : module->get_gates())
            createChildItem(g->get_id(), ModuleItem::TreeItemType::Gate, moduleItem);
    }

    void ModuleModel::removeModule(const u32 id)
    {
        auto it = mModuleMap.lowerBound(id);
        while (it != mModuleMap.upperBound(id))
        {
            ModuleItem* item   = mModuleMap.value(id);
            BaseTreeItem* parentItem = item->getParent();

            removeChildItem(item,parentItem);

            it = mModuleMap.erase(it);
        }
    }

    void ModuleModel::removeGate(const u32 id)
    {
        auto it = mGateMap.lowerBound(id);
        while (it != mGateMap.upperBound(id))
        {
            ModuleItem* item   = it.value();
            BaseTreeItem* parentItem = item->getParent();

            removeChildItem(item, parentItem);

            it = mGateMap.erase(it);
        }
    }

    void ModuleModel::removeNet(const u32 id)
    {
        auto it = mNetMap.lowerBound(id);
        while (it != mNetMap.upperBound(id))
        {
            ModuleItem* item   = it.value();
            BaseTreeItem* parentItem = item->getParent();

            removeChildItem(item, parentItem);

            it = mNetMap.erase(it);
        }
    }

    void ModuleModel::removeChildItem(ModuleItem *itemToRemove, BaseTreeItem *parentItem)
    {
        Q_ASSERT(itemToRemove);
        Q_ASSERT(parentItem);

        while (itemToRemove->getChildCount())
        {
            ModuleItem* childItem = static_cast<ModuleItem*>(itemToRemove->getChildren().at(0));
            int ityp = static_cast<int>(childItem->getType());
            auto it = mModuleItemMaps[ityp]->lowerBound(childItem->id());
            while (it != mModuleItemMaps[ityp]->upperBound(childItem->id()))
            {
                if (it.value() == childItem)
                    it = mModuleItemMaps[ityp]->erase(it);
                else
                    ++it;
            }
            removeChildItem(childItem,itemToRemove);
        }

        QModelIndex index = getIndexFromItem(parentItem);

        int row = itemToRemove->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parentItem->removeChild(itemToRemove);
        endRemoveRows();
        mIsModifying = false;

        delete itemToRemove;
    }

    void ModuleModel::handleModuleNameChanged(Module* mod)
    {
        updateModuleName(mod->get_id());
    }

    void ModuleModel::handleModuleRemoved(Module* mod)
    {
        removeModule(mod->get_id());
    }

    void ModuleModel::handleModuleCreated(Module* mod)
    {
        if (mod->get_parent_module() == nullptr) return;
        addModule(mod->get_id(), mod->get_parent_module()->get_id());
    }

    void ModuleModel::handleModuleGateAssigned(Module* mod, u32 gateId)
    {
        if (mTempGateAssignment.isAccumulate())
            mTempGateAssignment.assignGateToModule(gateId,mod);
        else
        {
            moduleAssignGate(mod->get_id(), gateId);
            moduleAssignNets({gateId});
        }
    }

    void ModuleModel::handleModuleGateRemoved(Module* mod, u32 gateId)
    {
        if (mTempGateAssignment.isAccumulate())
            mTempGateAssignment.removeGateFromModule(gateId,mod);
        else
        {
            auto it = mGateMap.lowerBound(gateId);
            while (it != mGateMap.upperBound(gateId))
            {
                ModuleItem* item   = it.value();
                if (!item->isToplevelItem())
                {
                    ModuleItem* parentItem = static_cast<ModuleItem*>(item->getParent());
                    if (parentItem->id() == mod->get_id())
                    {
                        removeChildItem(item, parentItem);
                        it = mGateMap.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }
    }

    void ModuleModel::handleModuleGatesAssignBegin(Module* mod, u32 numberGates)
    {
        Q_UNUSED(mod);
        Q_UNUSED(numberGates);
        mTempGateAssignment.beginAccumulate();
    }

    void ModuleModel::handleModuleGatesAssignEnd(Module* mod, u32 numberGates)
    {
        Q_UNUSED(mod);
        Q_UNUSED(numberGates);
        mTempGateAssignment.endAccumulate();
        if (!mTempGateAssignment.isAccumulate())
        {
            for (auto it = mTempGateAssignment.mGateAssign.begin(); it != mTempGateAssignment.mGateAssign.end(); ++it)
            {
                moduleAssignGate(it.value()->get_id(), it.key()); // moduleId, gateId
            }
            moduleAssignNets(mTempGateAssignment.mGateAssign.keys());
            mTempGateAssignment.mGateAssign.clear();
            mTempGateAssignment.mGateRemove.clear();
        }
    }

    void ModuleModel::handleGateRemoved(Gate* gat)
    {
        removeGate(gat->get_id());
    }

    void ModuleModel::handleGateCreated(Gate* gat)
    {
        Module* mod = gat->get_module();
        if (mod) moduleAssignGate(mod->get_id(), gat->get_id());
    }

    void ModuleModel::handleGateNameChanged(Gate* gat)
    {
        updateGateName(gat->get_id());
    }

    void ModuleModel::handleNetCreated(Net* net)
    {
        addNet(net->get_id(), gNetlist->get_top_module()->get_id());
    }

    void ModuleModel::handleNetRemoved(Net* net)
    {
        removeNet(net->get_id());
    }

    void ModuleModel::handleNetNameChanged(Net* net)
    {
        updateNetName(net->get_id());
    }

    void ModuleModel::handleNetUpdated(Net* net, u32 data)
    {
        Q_UNUSED(data);
        updateNetParent(net);
    }

    void ModuleModel::handleModuleParentChanged(const Module* mod)
    {
        Q_ASSERT(mod);
        updateModuleParent(mod);

        QHash<const Net*,ModuleItem*> parentAssignment;
        std::unordered_set<Net*> assignedNets;
        findNetParentRecursion(mRootItem, parentAssignment, assignedNets);

        for(Net* net : mod->get_nets())
            updateNetParent(net, &parentAssignment);
    }

    void ModuleModel::handleModuleSubmoduleAdded(Module* mod, u32 submodId)
    {
        Q_UNUSED(mod);
        Q_UNUSED(submodId);
    }

    void ModuleModel::handleModuleSubmoduleRemoved(Module* mod, u32 submodId)
    {
        Q_UNUSED(mod);
        Q_UNUSED(submodId);
    }

    void ModuleModel::findNetParentRecursion(BaseTreeItem* parent, QHash<const Net *, ModuleItem *> &parentAssignment, std::unordered_set<Net*>& assignedNets) const
    {
        for (BaseTreeItem* bti : parent->getChildren())
        {
            ModuleItem* item = dynamic_cast<ModuleItem*>(bti);
            if (!item || item->getType() != ModuleItem::TreeItemType::Module) continue;
            findNetParentRecursion(item, parentAssignment, assignedNets);
            Module* m = gNetlist->get_module_by_id(item->id());
            Q_ASSERT(m);
            std::unordered_set<Net*> internalNets = m->get_nets();
            if (!internalNets.empty())
            {
                for (Net* n : assignedNets)
                    internalNets.erase(n);
                for (Net* n : m->get_input_nets())
                    internalNets.erase(n);
                for (Net* n : m->get_output_nets())
                    internalNets.erase(n);
            }
            for (Net* n : internalNets)
            {
                parentAssignment[n] = item;
                assignedNets.insert(n);
            }
        }
    }

    Module* ModuleModel::findNetParent(const Net *net) const
    {
        QHash<Module*,int> modHash;
        if (net->is_global_input_net() || net->is_global_output_net()) return nullptr;
        int maxDepth = 0;

        for (const Endpoint* ep : net->get_sources())
        {
            Module* m = ep->get_gate()->get_module();
            Q_ASSERT(m);
            int depth = m->get_submodule_depth();
            if (depth > maxDepth) maxDepth = depth;
            modHash.insert(m,depth);
        }

        for (const Endpoint* ep : net->get_destinations())
        {
            Module* m = ep->get_gate()->get_module();
            Q_ASSERT(m);
            int depth = m->get_submodule_depth();
            if (depth > maxDepth) maxDepth = depth;
            modHash.insert(m,depth);
        }

        while (modHash.size() > 1 && maxDepth > 0)
        {
            auto it = modHash.begin();
            while (it != modHash.end())
            {
                if (it.value() == maxDepth)
                {
                    Module* parentMod = it.key()->get_parent_module();
                    modHash.erase(it);
                    if (parentMod) modHash.insert(parentMod,maxDepth-1);
                    break;
                }
                ++it;
            }
            if (it == modHash.end())
                --maxDepth;
        }
        if (modHash.empty()) return nullptr;
        return modHash.begin().key();
    }

    void ModuleModel::moduleAssignGate(const u32 moduleId, const u32 gateId)
    {
        // Don't need new function handleModuleGateRemoved(), because the GateAssinged event always follows GateRemoved
        // or NetlistInternalManager updates Net connections when a gate is deleted.

        QSet<ModuleItem*> parentsHandled;
        Q_ASSERT(gNetlist->get_gate_by_id(gateId));

        auto itGat = mGateMap.lowerBound(gateId);
        while (itGat != mGateMap.upperBound(gateId))
        {
            ModuleItem* gatItem = itGat.value();
            if (gatItem->isToplevelItem()) continue;
            ModuleItem* oldParentItem = static_cast<ModuleItem*>(gatItem->getParent());
            Q_ASSERT(oldParentItem);

            if (oldParentItem->id() != moduleId)
            {
                removeChildItem(gatItem,oldParentItem);
                itGat = mGateMap.erase(itGat);
            }
            else
            {
                parentsHandled.insert(oldParentItem);
                ++itGat;
            }

        }

        if (!moduleId) return;
        for (auto itMod = mModuleMap.lowerBound(moduleId); itMod != mModuleMap.upperBound(moduleId); ++itMod)
        {
            ModuleItem* parentItem = itMod.value();
            if (parentsHandled.contains(parentItem)) continue;
            createChildItem(gateId, ModuleItem::TreeItemType::Gate, parentItem);
        }

    }

    void ModuleModel::moduleAssignNets(const QList<u32>& gateIds)
    {
        QHash<const Net*,ModuleItem*> parentAssignment;
        std::unordered_set<Net*> assignedNets;
        findNetParentRecursion(mRootItem, parentAssignment, assignedNets);

        QSet<const Net*> netsToAssign;
        if (gateIds.isEmpty())
        {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            netsToAssign = QSet(parentAssignment.keys().begin(),parentAssignment.keys().end());
#else
            netsToAssign = parentAssignment.keys().toSet();
#endif
        }
        else
        {
            for (u32 id : gateIds)
            {
                Gate* gate = gNetlist->get_gate_by_id(id);
                for(Net* in_net : gate->get_fan_in_nets())
                    netsToAssign.insert(in_net);
                for(Net* out_net : gate->get_fan_out_nets())
                    netsToAssign.insert(out_net);
            }
        }

        for (const Net* n: netsToAssign)
            updateNetParent(n, &parentAssignment);
    }

    void ModuleModel::updateNetParent(const Net* net, const QHash<const Net *, ModuleItem *> *parentAssignment)
    {
        Q_ASSERT(net);
        u32 netId = net->get_id();

        QSet<ModuleItem*> parentsHandled;
        u32 newParentId = 0;
        if (parentAssignment)
        {
            ModuleItem* modItem = parentAssignment->value(net);
            if (modItem)
                newParentId = modItem->id();
        }
        else
        {
            Module* newParentModule = findNetParent(net);
            if (newParentModule)
                newParentId = newParentModule->get_id();
        }

        auto itNet = mNetMap.lowerBound(netId);
        while (itNet != mNetMap.upperBound(netId))
        {
            if (itNet.value()->isToplevelItem()) continue;
            
            ModuleItem* netItem = itNet.value();
            ModuleItem* oldParentItem = static_cast<ModuleItem*>(netItem->getParent());
            Q_ASSERT(oldParentItem);


            if (newParentId == 0 || newParentId != oldParentItem->id())
            {
                removeChildItem(netItem,oldParentItem);
                itNet = mNetMap.erase(itNet);
            }
            else
            {
                parentsHandled.insert(oldParentItem);
                ++itNet;
            }
        }

        if (!newParentId) return;
        for (auto itMod = mModuleMap.lowerBound(newParentId); itMod != mModuleMap.upperBound(newParentId); ++itMod)
        {
            ModuleItem* parentItem = itMod.value();
            if (parentsHandled.contains(parentItem)) continue;
            createChildItem(net->get_id(), ModuleItem::TreeItemType::Net, parentItem);
        }
    }

    void ModuleModel::updateModuleParent(const Module* module)
    {
        ModuleItem* moduleItemToBeMoved = nullptr;
        bool moduleItemReassigned = false;

        Q_ASSERT(module);
        u32 id = module->get_id();
        Q_ASSERT(id != 1);

        QSet<ModuleItem*> parentsHandled;
        u32 parentId = module->get_parent_module()->get_id();
        Q_ASSERT(parentId > 0);

        auto itSubm = mModuleMap.lowerBound(id);
        while (itSubm != mModuleMap.upperBound(id))
        {
            ModuleItem* submItem = itSubm.value();
            if (submItem->isToplevelItem()) continue;
            ModuleItem* oldParentItem = static_cast<ModuleItem*>(submItem->getParent());
            Q_ASSERT(oldParentItem);

            if (oldParentItem->id() != parentId)
            {
                if (moduleItemToBeMoved)
                {
                    // remove tree item recursively
                    removeChildItem(submItem,oldParentItem);
                    itSubm = mModuleMap.erase(itSubm);
                }
                else
                {
                    // save tree item for reassignment
                    moduleItemToBeMoved = submItem;
                    QModelIndex index = getIndexFromItem(oldParentItem);

                    int row = submItem->row();

                    mIsModifying = true;
                    beginRemoveRows(index, row, row);
                    oldParentItem->removeChild(submItem);
                    endRemoveRows();
                    mIsModifying = false;
                    ++itSubm;
                }
            }
            else
            {
                parentsHandled.insert(oldParentItem);
                ++itSubm;
            }
        }

        if (!parentId) return;
        for (auto itMod = mModuleMap.lowerBound(parentId); itMod != mModuleMap.upperBound(parentId); ++itMod)
        {
            ModuleItem* parentItem = itMod.value();
            if (parentsHandled.contains(parentItem)) continue;
            if (moduleItemToBeMoved && !moduleItemReassigned)
            {
                QModelIndex index = getIndexFromItem(parentItem);
                int row = parentItem->getChildCount();
                mIsModifying = true;
                beginInsertRows(index, row, row);
                parentItem->appendChild(moduleItemToBeMoved);
                endInsertRows();
                mIsModifying = false;
                moduleItemReassigned = true;
            }
            else
            {
                addRecursively(module, parentItem);
            }
        }

        if (moduleItemToBeMoved && !moduleItemReassigned)
        {
            // stored item could not be reassigned, delete it
            auto it = mModuleMap.lowerBound(id);
            while (it != mModuleMap.upperBound(id))
            {
                if (it.value() == moduleItemToBeMoved)
                    it = mModuleMap.erase(it);
                else
                    ++it;
            }
            delete moduleItemToBeMoved;
        }

    }

    void ModuleModel::updateModuleName(u32 id)
    {
        Q_ASSERT(gNetlist->get_module_by_id(id));

        for (auto it = mModuleMap.lowerBound(id); it != mModuleMap.upperBound(id); ++it)
        {
            ModuleItem* item = it.value();
            Q_ASSERT(item);

            item->setName(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

            QModelIndex index = getIndexFromItem(item);
            Q_EMIT dataChanged(index, index);
        }
    }

    void ModuleModel::updateGateName(u32 id)
    {
        Q_ASSERT(gNetlist->get_gate_by_id(id));

        for (auto it = mGateMap.lowerBound(id); it != mGateMap.upperBound(id); ++it)
        {
            ModuleItem* item = it.value();
            Q_ASSERT(item);

            item->setName(QString::fromStdString(gNetlist->get_gate_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

            QModelIndex index = getIndexFromItem(item);
            Q_EMIT dataChanged(index, index);
        }
    }

    void ModuleModel::updateNetName(u32 id)
    {
        Q_ASSERT(gNetlist->get_net_by_id(id));
        Q_ASSERT(mNetMap.contains(id));

        ModuleItem* item = mNetMap.value(id);
        Q_ASSERT(item);
        
        item->setName(QString::fromStdString(gNetlist->get_net_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

        QModelIndex index = getIndexFromItem(item);
        Q_EMIT dataChanged(index, index);
    }

    ModuleItem* ModuleModel::getItem(u32 id, ModuleItem::TreeItemType type) const
    {
        return mModuleItemMaps[(int)type]->value(id);
    }

    ModuleItem* ModuleModel::createChildItem(u32 id, ModuleItem::TreeItemType itemType, BaseTreeItem *parentItem)
    {
        ModuleItem* retval = new ModuleItem(id, itemType);
        mModuleItemMaps[(int)itemType]->insertMulti(id,retval);

        if (!parentItem) parentItem = mRootItem;
        QModelIndex index = getIndexFromItem(parentItem);
        int row = parentItem->getChildCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parentItem->appendChild(retval);
        endInsertRows();
        mIsModifying = false;

        return retval;
    }


    bool ModuleModel::isModifying()
    {
        return mIsModifying;
    }
    
}
