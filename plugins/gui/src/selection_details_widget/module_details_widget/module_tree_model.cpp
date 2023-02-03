#include "gui/selection_details_widget/module_details_widget/module_tree_model.h"
#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/basic_tree_model/tree_item.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QQueue>

namespace hal
{

    ModuleTreeModel::ModuleTreeModel(QObject* parent) : BaseTreeModel(parent), mModId(-1)
    {
        setHeaderLabels(QList<QVariant>() << "Name" << "ID" << "Type");
    }

    ModuleTreeModel::~ModuleTreeModel()
    {

    }

    void ModuleTreeModel::setModule(Module *m)
    {
        clearOwnStructures();
        if(!m)
        {
            clear();
            disconnectEvents();
            return;
            //disconnect all events?
        }
        beginResetModel();
        //delete all children, not the root item (manually for performance reasons)
        while(mRootItem->getChildCount() > 0)
        {
            TreeItem* tmp = mRootItem->removeChildAtPos(0);
            delete tmp;
        }

        mModId = m->get_id();
        //add modules
        for(auto mod : m->get_submodules())
        {
            TreeItem* modItem = new TreeItem(QList<QVariant>() << QString::fromStdString(mod->get_name())
                                                << mod->get_id() << QString::fromStdString(mod->get_type()));
            moduleRecursive(mod, modItem);
            modItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::module));
            modItem->setAdditionalData(mKeyRepId, mod->get_id());
            mRootItem->appendChild(modItem);
            mModuleToTreeitems.insert(mod, modItem);
        }
        //add gates
        for(auto gate : m->get_gates())
        {
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                              << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
            gateItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(mKeyRepId, gate->get_id());
            mRootItem->appendChild(gateItem);
            mGateToTreeitems.insert(gate, gateItem);
        }
        endResetModel();

        if(!mEventsConnected)
            connectEvents();

        Q_EMIT numberOfSubmodulesChanged(m->get_submodules().size());
    }

    void ModuleTreeModel::clear()
    {
        BaseTreeModel::clear();
        clearOwnStructures();
    }

    QVariant ModuleTreeModel::data(const QModelIndex &index, int role) const
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

    ModuleTreeModel::itemType ModuleTreeModel::getTypeOfItem(TreeItem *item) const
    {
        return item->getAdditionalData(mKeyItemType).value<itemType>();
    }

    void ModuleTreeModel::moduleRecursive(Module *mod, TreeItem *modItem)
    {
        TreeItem* subModItem = nullptr;
        for(Module* subMod : mod->get_submodules())
        {
            subModItem = new TreeItem(QList<QVariant>() << QString::fromStdString(subMod->get_name())
                                                << subMod->get_id() << QString::fromStdString(subMod->get_type()));
            moduleRecursive(subMod, subModItem);
            subModItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::module));
            subModItem->setAdditionalData(mKeyRepId, subMod->get_id());
            modItem->appendChild(subModItem);
            mModuleToTreeitems.insert(subMod, subModItem);
        }
        for(auto gate : mod->get_gates())
        {
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                              << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
            gateItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::gate));
            gateItem->setAdditionalData(mKeyRepId, gate->get_id());
            modItem->appendChild(gateItem);
            mGateToTreeitems.insert(gate, gateItem);
        }
    }

    void ModuleTreeModel::updateGatesOfModule(Module* mod)
    {
        auto modItem = mModuleToTreeitems.value(mod, nullptr);
        if((int)mod->get_id() == mModId)
            modItem = mRootItem;
        if(!modItem)
            return;

        //1. Find index of first gate-type item
        int startIndex = 0;
        for(; startIndex < modItem->getChildCount(); startIndex++)
            if(getTypeOfItem(modItem->getChild(startIndex)) != itemType::module)
                break;

        beginResetModel();

        //2. Check if removing of gates is necessary, if yes remove them
        if(startIndex < modItem->getChildCount())
        {
            while(modItem->getChildCount() > startIndex)
            {
                auto child = modItem->removeChildAtPos(modItem->getChildCount()-1);
                auto gate = gNetlist->get_gate_by_id(child->getData(sIdColumn).toInt());
                mGateToTreeitems.remove(gate);
                delete child;
            }
        }

        //3. Check if adding of gates is necessary, if yes add them
        if(!mod->get_gates().empty())
        {
            beginResetModel();
            for(auto gate : mod->get_gates())
            {
                TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                                  << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
                gateItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::gate));
                gateItem->setAdditionalData(mKeyRepId, gate->get_id());
                modItem->appendChild(gateItem);
                mGateToTreeitems.insert(gate, gateItem);
            }
        }
        endResetModel();
    }

    QIcon ModuleTreeModel::getIconFromItem(TreeItem *item) const
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

    void ModuleTreeModel::clearOwnStructures()
    {
        mGateToTreeitems.clear();
        mModuleToTreeitems.clear();
        mModId = -1;
    }

    void ModuleTreeModel::handleModuleGatesAssignBegin(Module *m, u32 associated_data)
    {
        if((int)associated_data <= mThreshold)
            return;

        if(mModuleToTreeitems.value(m, nullptr) || (int)m->get_id() == mModId)
            disconnect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
    }

    void ModuleTreeModel::handleModuleGatesAssignEnd(Module *m, u32 associated_data)
    {
        Q_UNUSED(associated_data)
        if((int)associated_data <= mThreshold)
            return;

        if(mModuleToTreeitems.value(m, nullptr) || (mModId == (int)m->get_id() && gNetlist->get_module_by_id(mModId)))
        {
            connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
            updateGatesOfModule(m);
        }
    }

    void ModuleTreeModel::handleModuleGatesRemoveBegin(Module *m, u32 associated_data)
    {
        if((int)associated_data <= mThreshold)
            return;

        if(mModuleToTreeitems.value(m, nullptr) || (int)m->get_id() == mModId)
            disconnect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
    }

    void ModuleTreeModel::handleModuleGatesRemoveEnd(Module *m, u32 associated_data)
    {
        if((int)associated_data <= mThreshold)
            return;

        if(mModuleToTreeitems.value(m, nullptr) || (mModId == (int)m->get_id() && gNetlist->get_module_by_id(mModId)))
        {
            connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
            updateGatesOfModule(m);
        }
    }

    void ModuleTreeModel::handleModuleSubmoduleAdded(Module *m, u32 added_module)
    {
        auto parentModItem = mModuleToTreeitems.value(m, nullptr);
        if(parentModItem || (int)m->get_id() == mModId)
        {
            beginResetModel();
            auto addedMod = gNetlist->get_module_by_id(added_module);
            TreeItem* addedSubmodItem = new TreeItem(QList<QVariant>() << QString::fromStdString(addedMod->get_name()) << addedMod->get_id()
                                                     << QString::fromStdString(addedMod->get_type()));
            moduleRecursive(addedMod, addedSubmodItem);
            addedSubmodItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::module));
            addedSubmodItem->setAdditionalData(mKeyRepId, addedMod->get_id());
            parentModItem ? parentModItem->insertChild(0, addedSubmodItem) : mRootItem->insertChild(0, addedSubmodItem);
            mModuleToTreeitems.insert(addedMod, addedSubmodItem);
            endResetModel();
        }
    }

    void ModuleTreeModel::handleModuleSubmoduleRemoved(Module *m, u32 removed_module)
    {
        Q_UNUSED(m)

        auto removedModItem = mModuleToTreeitems.value(gNetlist->get_module_by_id(removed_module), nullptr);
        if(!removedModItem)
            return;

        //1. Remove all items from maps through BFS (maybe own function?)
        QQueue<TreeItem*> treeItemsQueue;
        treeItemsQueue.enqueue(removedModItem);
        while(!treeItemsQueue.isEmpty())
        {
            TreeItem* current = treeItemsQueue.dequeue();
            switch (getTypeOfItem(current))
            {
                case itemType::module: mModuleToTreeitems.remove(gNetlist->get_module_by_id(current->getData(ModuleTreeModel::sIdColumn).toInt())); break;
                case itemType::gate: mGateToTreeitems.remove(gNetlist->get_gate_by_id(current->getData(ModuleTreeModel::sIdColumn).toInt()));break;
            }
            for(auto child : current->getChildren())
                treeItemsQueue.enqueue(child);
        }

        //2. Delete item, reset model
        //beginRemoveRows(parent(getIndexFromItem(removedModItem)), removedModItem->getOwnRow(), removedModItem->getOwnRow());
        beginResetModel();
        removedModItem->getParent()->removeChild(removedModItem);
        delete removedModItem;
        endResetModel();
        //endRemoveRows();
    }

    void ModuleTreeModel::handleModuleGateAssigned(Module *m, u32 assigned_gate)
    {
        TreeItem* modItem = mModuleToTreeitems.value(m, nullptr);
        if((int)m->get_id() == mModId)
            modItem = mRootItem;

        if(!modItem)
            return;

        auto assignedGate = gNetlist->get_gate_by_id(assigned_gate);
        int indexToInsert = 0; //first item after the modules
        for(; indexToInsert < modItem->getChildCount(); indexToInsert++)
            if(getTypeOfItem(modItem->getChild(indexToInsert)) != itemType::module)
                break;

        TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(assignedGate->get_name())
                                          << assignedGate->get_id() << QString::fromStdString(assignedGate->get_type()->get_name()));
        gateItem->setAdditionalData(mKeyItemType, QVariant::fromValue(itemType::gate));
        mGateToTreeitems.insert(assignedGate, gateItem);
        //beginInsertRows(getIndexFromItem(modItem), indexToInsert, indexToInsert);
        beginResetModel();
        modItem->insertChild(indexToInsert, gateItem);
        endResetModel();
        //endInsertRows();
    }

    void ModuleTreeModel::handleModuleGateRemoved(Module *m, u32 removed_gate)
    {
        Q_UNUSED(m)
        //only works if the gate is first removed from the module, then added to another (otherwise wrong one is removed)
        auto gate = gNetlist->get_gate_by_id(removed_gate);
        auto gateItem = mGateToTreeitems.value(gate);
        if(!gateItem)
            return;

        //beginRemoveRows(parent(getIndexFromItem(gateItem)), gateItem->getOwnRow(), gateItem->getOwnRow());
        beginResetModel();
        mGateToTreeitems.remove(gate);
        gateItem->getParent()->removeChild(gateItem);
        delete gateItem;
        endResetModel();
        //endRemoveRows();
    }

    void ModuleTreeModel::handleModuleRemoved(Module *m)
    {
        if((int)m->get_id() == mModId)
            clear();
    }

    void ModuleTreeModel::handleGateNameChanged(Gate *g)
    {
        auto gateItem = mGateToTreeitems.value(g, nullptr);
        if(gateItem)
        {
            gateItem->setDataAtIndex(sNameColumn, QString::fromStdString(g->get_name()));
            QModelIndex inx0 = getIndexFromItem(gateItem);
            QModelIndex inx1 = createIndex(inx0.row(), sNameColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void ModuleTreeModel::handleModuleNameChanged(Module *m)
    {
        auto moduleItem = mModuleToTreeitems.value(m, nullptr);
        if(moduleItem)
        {
            moduleItem->setDataAtIndex(sNameColumn, QString::fromStdString(m->get_name()));
            QModelIndex inx0 = getIndexFromItem(moduleItem);
            QModelIndex inx1 = createIndex(inx0.row(), sNameColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void ModuleTreeModel::handleModuleTypeChanged(Module *m)
    {
        auto moduleItem = mModuleToTreeitems.value(m, nullptr);
        if(moduleItem)
        {
            moduleItem->setDataAtIndex(sTypeColumn, QString::fromStdString(m->get_type()));
            QModelIndex inx0 = getIndexFromItem(moduleItem);
            QModelIndex inx1 = createIndex(inx0.row(), sTypeColumn, inx0.internalPointer());
            Q_EMIT dataChanged(inx0, inx1);
        }
    }

    void ModuleTreeModel::disconnectEvents()
    {
        //guards
        disconnect(gNetlistRelay, &NetlistRelay::moduleGatesAssignEnd, this, &ModuleTreeModel::handleModuleGatesAssignEnd);
        disconnect(gNetlistRelay, &NetlistRelay::moduleGatesAssignBegin, this, &ModuleTreeModel::handleModuleGatesAssignBegin);
        disconnect(gNetlistRelay, &NetlistRelay::moduleGatesRemoveBegin, this, &ModuleTreeModel::handleModuleGatesRemoveBegin);
        disconnect(gNetlistRelay, &NetlistRelay::moduleGatesRemoveEnd, this, &ModuleTreeModel::handleModuleGatesRemoveEnd);

        //actual events
        disconnect(gNetlistRelay, &NetlistRelay::moduleSubmoduleAdded, this, &ModuleTreeModel::handleModuleSubmoduleAdded);
        disconnect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &ModuleTreeModel::handleModuleSubmoduleRemoved);

        disconnect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
        disconnect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);

        disconnect(gNetlistRelay, &NetlistRelay::moduleRemoved, this, &ModuleTreeModel::handleModuleRemoved);

        //information change
        disconnect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &ModuleTreeModel::handleGateNameChanged);
        disconnect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &ModuleTreeModel::handleModuleNameChanged);
        disconnect(gNetlistRelay, &NetlistRelay::moduleTypeChanged, this, &ModuleTreeModel::handleModuleTypeChanged);

        mEventsConnected = false;
    }

    void ModuleTreeModel::connectEvents()
    {
        //guards
        connect(gNetlistRelay, &NetlistRelay::moduleGatesAssignEnd, this, &ModuleTreeModel::handleModuleGatesAssignEnd);
        connect(gNetlistRelay, &NetlistRelay::moduleGatesAssignBegin, this, &ModuleTreeModel::handleModuleGatesAssignBegin);
        connect(gNetlistRelay, &NetlistRelay::moduleGatesRemoveBegin, this, &ModuleTreeModel::handleModuleGatesRemoveBegin);
        connect(gNetlistRelay, &NetlistRelay::moduleGatesRemoveEnd, this, &ModuleTreeModel::handleModuleGatesRemoveEnd);

        //actual events
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleAdded, this, &ModuleTreeModel::handleModuleSubmoduleAdded);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &ModuleTreeModel::handleModuleSubmoduleRemoved);

        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);

        connect(gNetlistRelay, &NetlistRelay::moduleRemoved, this, &ModuleTreeModel::handleModuleRemoved);

        //information change
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &ModuleTreeModel::handleGateNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &ModuleTreeModel::handleModuleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleTypeChanged, this, &ModuleTreeModel::handleModuleTypeChanged);

        mEventsConnected = true;
    }
}
