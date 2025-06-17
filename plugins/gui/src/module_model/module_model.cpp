#include "gui/module_model/module_model.h"

#include "gui/gui_globals.h"

#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/python/py_code_provider.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include <QMimeData>

namespace hal
{
    ModuleModel::ModuleModel(QObject* parent) : BaseTreeModel(parent)
    {
        // use root item to store header information
        setHeaderLabels(QStringList() << "Name" << "ID" << "Type");
        connect(gNetlistRelay, &NetlistRelay::moduleCreated,          this, &ModuleModel::handleModuleCreated);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged,      this, &ModuleModel::handleModuleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleTypeChanged,      this, &ModuleModel::handleModuleTypeChanged);
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
            case Qt::TextAlignmentRole:
                return index.column() == 1
                        ? Qt::AlignRight
                        : Qt::AlignLeft;
            default:
                return QVariant();
        }
        return QVariant();
    }

    QMimeData* ModuleModel::mimeData(const QModelIndexList &indexes) const
    {
        QMimeData* retval = new QMimeData;
        // only single row allowed
        int row = -1;
        for (const QModelIndex& inx : indexes)
        {
            if (row < 0)
                row = inx.row();
            else if (row != inx.row())
                return retval;
        }
        if (row < 0)
            return retval;

        QModelIndex firstIndex = indexes.at(0);
        BaseTreeItem* bti = getItemFromIndex(firstIndex);
        row = firstIndex.row();
        BaseTreeItem* parentItem = bti->getParent();
        ModuleItem* item = dynamic_cast<ModuleItem*>(bti);
        if (!item)
        {
            qDebug() << "cannot cast" << indexes.at(0);
            return retval;
        }
        QByteArray encodedData;
        QDataStream stream(&encodedData, QIODevice::WriteOnly);
        QString moveText;
        int id = item->id();

        switch (item->getType())
        {
        case ModuleItem::TreeItemType::Module:
            moveText = PyCodeProvider::pyCodeModule(id);
            break;
        case ModuleItem::TreeItemType::Gate:
            moveText = PyCodeProvider::pyCodeGate(id);
            break;
        case ModuleItem::TreeItemType::Net:
            moveText = PyCodeProvider::pyCodeNet(id);
            break;
        }

        stream << item->getType() << id << row << (quintptr) parentItem;
        retval->setText(moveText);
        retval->setData("modulemodel/item", encodedData);
        return retval;

    }

    Qt::ItemFlags ModuleModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
    }

    ModuleItem* ModuleModel::getItem(const QModelIndex& index) const
    {
        if (index.isValid())
            return static_cast<ModuleItem*>(index.internalPointer());
        else
            return nullptr;
    }

    ModuleItem* ModuleModel::getItem(u32 id, ModuleItem::TreeItemType type) const
    {
        return mModuleItemMaps[(int)type]->value(id);
    }

    QList<ModuleItem*> ModuleModel::getItems(u32 id, ModuleItem::TreeItemType type) const
    {
        return mModuleItemMaps[(int)type]->values(id);
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

    void ModuleModel::populateFromGatelist(const std::vector<Gate *> &gates)
    {
        setIsModifying(true);
        beginResetModel();
        clear();

        QMap<Module*,ModuleItem*> parentMap;
        for (const Gate* g : gates)
        {
            Module* parentModule = g->get_module();
            ModuleItem* parentItem;
            bool insertToRoot = true;
            ModuleItem* childItem = new ModuleItem(g->get_id(), ModuleItem::TreeItemType::Gate, this);

            while (parentModule && insertToRoot)
            {
                parentItem = parentMap.value(parentModule);
                if (!parentItem)
                {
                    parentItem = new ModuleItem(parentModule->get_id(), ModuleItem::TreeItemType::Module, this);
                    parentMap.insert(parentModule, parentItem);
                }
                else
                {
                    insertToRoot = false;
                }
                parentItem->appendChild(childItem);
                parentModule = parentModule->get_parent_module();
                childItem = parentItem;
            }

            if (insertToRoot)
            {
                mRootItem->appendChild(parentItem);
            }
        }

        setIsModifying(false);
        endResetModel();
    }

    void ModuleModel::populateTree(const QVector<u32>& modIds, const QVector<u32>& gateIds, const QVector<u32>& netIds)
    {
        setIsModifying(true);
        beginResetModel();
        // Might want to add parameter for container of moduleIds that don't get recursively inserted.
        clear();

        QList<ModuleItem*> newRootList;
        for(u32 id : modIds)
            addRecursively(gNetlist->get_module_by_id(id));
        moduleAssignNets();

        for(u32 id : gateIds)
            newRootList.append(new ModuleItem(id, ModuleItem::TreeItemType::Gate, this));
        
        for(u32 id : netIds)
            newRootList.append(new ModuleItem(id, ModuleItem::TreeItemType::Net, this));

        for(auto item : newRootList)
            mRootItem->appendChild(item);
        setIsModifying(false);
        endResetModel();
    }

    void ModuleModel::addModule(u32 id, u32 parentId)
    {
        Q_ASSERT(gNetlist->get_module_by_id(id));
        Q_ASSERT(gNetlist->get_module_by_id(parentId));

        for (auto it = mModuleMap.lowerBound(parentId); it != mModuleMap.upperBound(parentId); ++it)
        {
            ModuleItem* parentItem = it.value();
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
        QList<ModuleItem*> trashcan;  // access items to delete without map

        for (auto it = mModuleMap.lowerBound(id); it != mModuleMap.upperBound(id); ++it)
            trashcan.append(it.value());

        for (ModuleItem* item : trashcan)
        {
            BaseTreeItem* parentItem = item->getParent();
            removeChildItem(item,parentItem);
        }
    }

    void ModuleModel::removeGate(const u32 id)
    {
        QList<ModuleItem*> trashcan;  // access items to delete without map

        for (auto it = mGateMap.lowerBound(id); it != mGateMap.upperBound(id); ++it)
            trashcan.append(it.value());

        for (ModuleItem* item : trashcan)
        {
            BaseTreeItem* parentItem = item->getParent();
            removeChildItem(item,parentItem);
        }
    }

    void ModuleModel::removeNet(const u32 id)
    {
        QList<ModuleItem*> trashcan;  // access items to delete without map

        for (auto it = mNetMap.lowerBound(id); it != mNetMap.upperBound(id); ++it)
            trashcan.append(it.value());

        for (ModuleItem* item : trashcan)
        {
            BaseTreeItem* parentItem = item->getParent();
            removeChildItem(item,parentItem);
        }
    }

    ModuleItem* ModuleModel::createChildItem(u32 id, ModuleItem::TreeItemType itemType, BaseTreeItem *parentItem)
    {
        ModuleItem* retval = new ModuleItem(id, itemType, this);

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

    void ModuleModel::removeChildItem(ModuleItem *itemToRemove, BaseTreeItem *parentItem)
    {
        Q_ASSERT(itemToRemove);
        Q_ASSERT(parentItem);

        while (itemToRemove->getChildCount())
        {
            ModuleItem* childItem = static_cast<ModuleItem*>(itemToRemove->getChildren().at(0));
            removeChildItem(childItem,itemToRemove);
        }

        QModelIndex index = getIndexFromItem(parentItem);

        int row = itemToRemove->getOwnRow();

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

    void ModuleModel::handleModuleTypeChanged(Module* mod)
    {
        updateModuleType(mod->get_id());
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
        QList<QPair<ModuleItem*,ModuleItem*> > trashcan;  // item to delete, parent

        if (mTempGateAssignment.isAccumulate())
            mTempGateAssignment.removeGateFromModule(gateId,mod);
        else
        {
            for (auto it = mGateMap.lowerBound(gateId); it != mGateMap.upperBound(gateId); ++it)
            {
                ModuleItem* item   = it.value();
                if (item->isToplevelItem()) continue;;

                ModuleItem* parentItem = static_cast<ModuleItem*>(item->getParent());
                if (parentItem->id() == mod->get_id())
                    trashcan.append(QPair<ModuleItem*,ModuleItem*>(item, parentItem));
            }
        }

        for (const QPair<ModuleItem*,ModuleItem*>& trash : trashcan)
            removeChildItem(trash.first, trash.second);
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

        QList<QPair<ModuleItem*,ModuleItem*> > trashcan;  // item to delete, parent
        for (auto itGat = mGateMap.lowerBound(gateId); itGat != mGateMap.upperBound(gateId); ++itGat)
        {
            ModuleItem* gatItem = itGat.value();
            if (gatItem->isToplevelItem()) continue;
            ModuleItem* oldParentItem = static_cast<ModuleItem*>(gatItem->getParent());
            Q_ASSERT(oldParentItem);

            if (oldParentItem->id() != moduleId)
            {
                trashcan.append(QPair<ModuleItem*,ModuleItem*>(gatItem,oldParentItem));
            }
            else
            {
                parentsHandled.insert(oldParentItem);
            }
        }
        for (const QPair<ModuleItem*,ModuleItem*>& trash : trashcan)
            removeChildItem(trash.first, trash.second);

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
            QList<const Net*> tempKeys = parentAssignment.keys();
            netsToAssign = QSet(tempKeys.begin(),tempKeys.end());
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

        QList<QPair<ModuleItem*,ModuleItem*> > trashcan;  // item to delete, parent

        for (auto itNet = mNetMap.lowerBound(netId); itNet != mNetMap.upperBound(netId); ++itNet)
        {
            if (itNet.value()->isToplevelItem()) continue;

            ModuleItem* netItem = itNet.value();
            ModuleItem* oldParentItem = static_cast<ModuleItem*>(netItem->getParent());
            Q_ASSERT(oldParentItem);

            if (newParentId == 0 || newParentId != oldParentItem->id())
            {
                trashcan.append(QPair<ModuleItem*,ModuleItem*>(netItem,oldParentItem));
                break;
            }
            else
            {
                parentsHandled.insert(oldParentItem);
            }
        }

        for (const QPair<ModuleItem*,ModuleItem*>& trash : trashcan)
            removeChildItem(trash.first, trash.second);

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

        QList<QPair<ModuleItem*,ModuleItem*> > trashcan;  // item to delete, parent

        for (auto itSubm = mModuleMap.lowerBound(id); itSubm != mModuleMap.upperBound(id); ++itSubm)
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
                    trashcan.append(QPair<ModuleItem*,ModuleItem*>(submItem,oldParentItem));
                }
                else
                {
                    // save tree item for reassignment
                    moduleItemToBeMoved = submItem;
                    QModelIndex index = getIndexFromItem(oldParentItem);

                    int row = submItem->getOwnRow();

                    mIsModifying = true;
                    beginRemoveRows(index, row, row);
                    oldParentItem->removeChild(submItem);
                    endRemoveRows();
                    mIsModifying = false;
                }
            }
            else
            {
                parentsHandled.insert(oldParentItem);
            }
        }
        for (const QPair<ModuleItem*,ModuleItem*>& trash : trashcan)
            removeChildItem(trash.first, trash.second);

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
            removeChildItem(moduleItemToBeMoved, moduleItemToBeMoved->getParent());
        }

    }

    void ModuleModel::updateModuleName(u32 id)
    {
        Q_ASSERT(gNetlist->get_module_by_id(id));

        for (auto it = mModuleMap.lowerBound(id); it != mModuleMap.upperBound(id); ++it)
        {
            ModuleItem* item = it.value();
            Q_ASSERT(item);

            item->setName(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name()));

            QModelIndex index = getIndexFromItem(item);
            Q_EMIT dataChanged(index, index);
        }
    }

    void ModuleModel::updateModuleType(u32 id)
    {
        Q_ASSERT(gNetlist->get_module_by_id(id));

        for (auto it = mModuleMap.lowerBound(id); it != mModuleMap.upperBound(id); ++it)
        {
            ModuleItem* item = it.value();
            Q_ASSERT(item);

            item->setModuleType(QString::fromStdString(gNetlist->get_module_by_id(id)->get_type()));

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

        // if net elment not in model loop will reject
        for (auto it = mNetMap.lowerBound(id); it != mNetMap.upperBound(id); ++it)
        {
            ModuleItem* item = it.value();
            Q_ASSERT(item);
        
            item->setName(QString::fromStdString(gNetlist->get_net_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

            QModelIndex index = getIndexFromItem(item);
            Q_EMIT dataChanged(index, index);
        }
    }

    bool ModuleModel::isModifying()
    {
        return mIsModifying;
    }

    void ModuleModel::setIsModifying(bool pIsModifying)
    {
        mIsModifying = pIsModifying;
    }
}
