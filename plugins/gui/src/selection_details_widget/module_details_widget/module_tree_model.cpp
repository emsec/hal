#include "gui/selection_details_widget/module_details_widget/module_tree_model.h"
#include "gui/basic_tree_model/tree_item.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QQueue>
#include <QDebug>

namespace hal
{

    ModuleTreeModel::ModuleTreeModel(QObject* parent) : BaseTreeModel(parent), mModId(-1), mModuleIcon(QIcon(":/icons/sel_module")), mGateIcon(QIcon(":/icons/sel_gate"))
    {
        setHeaderLabels(QList<QVariant>() << "Name" << "ID" << "Type");

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

        qDebug() << "In constructor";
    }

    ModuleTreeModel::~ModuleTreeModel()
    {

    }

    void ModuleTreeModel::setModule(Module *m)
    {
        qDebug() << "in setModule";

        clearOwnStructures();
        if(!m)
        {
            clear();
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
        clearOwnStructures();

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
        connectEvents();
        Q_EMIT numberOfSubmodulesChanged(m->get_submodules().size());
        //connect events in case of disconnections?
    }

    void ModuleTreeModel::clear()
    {
        BaseTreeModel::clear();
        clearOwnStructures();
        //disconnect all events?
        disconnectEvents();
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

    QIcon ModuleTreeModel::getIconFromItem(TreeItem *item) const
    {
        if(!item)
            return mGateIcon;//some cool gate icon if the item is a nullptr

        switch (getTypeOfItem(item))
        {
        case itemType::module: return mModuleIcon;
        case itemType::gate: return mGateIcon;
        default: return mGateIcon;
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
        qDebug() << "in modulegatesassignBEGIN";
        Q_UNUSED(associated_data)
        //disconnect relevant (or all) events
        if(mModuleToTreeitems.value(m, nullptr) || (int)m->get_id() == mModId)
        {
            disconnect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
        }
    }

    void ModuleTreeModel::handleModuleGatesAssignEnd(Module *m, u32 associated_data)
    {
        qDebug() << "in modulegatesassignEND";
        Q_UNUSED(associated_data)
        //connect relevant (or all) events
        //for now set module, otherwise update only specific module? (add only gates to module with beginInsertRows
        if(mModuleToTreeitems.value(m, nullptr) || (mModId == (int)m->get_id() && gNetlist->get_module_by_id(mModId)))
        {
            connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
            //need own guards?
            connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
            setModule(gNetlist->get_module_by_id(mModId));
        }
    }

    void ModuleTreeModel::handleModuleGatesRemoveBegin(Module *m, u32 associated_data)
    {
        Q_UNUSED(m)
        if((int)associated_data > mThreshold)
            disconnect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
    }

    void ModuleTreeModel::handleModuleGatesRemoveEnd(Module *m, u32 associated_data)
    {
        if((int)associated_data > mThreshold)
        {
            connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
            //OPTIMIZE: Only update specific Module (everywhere)
            if(mModuleToTreeitems.value(m, nullptr) || (int)m->get_id() == mModId)
                setModule(gNetlist->get_module_by_id(mModId));
        }
    }

    void ModuleTreeModel::handleModuleSubmoduleAdded(Module *m, u32 added_module)
    {
        qDebug() << "in new submodadded";
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
        qDebug() << "in submodremoved";

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

        //2. Delete item, reset model (better: beginRemoveRows)
        beginRemoveRows(parent(getIndexFromItem(removedModItem)), removedModItem->getOwnRow(), removedModItem->getOwnRow());
        removedModItem->getParent()->removeChild(removedModItem);
        delete removedModItem;
        endRemoveRows();
    }

    void ModuleTreeModel::handleModuleGateAssigned(Module *m, u32 assigned_gate)
    {
        qDebug() << "in gate assigned";
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
        beginInsertRows(parent(getIndexFromItem(modItem)), indexToInsert, indexToInsert);
        modItem->insertChild(indexToInsert, gateItem);
        endInsertRows();
    }

    void ModuleTreeModel::handleModuleGateRemoved(Module *m, u32 removed_gate)
    {
        Q_UNUSED(m)
        qDebug() << "in gateremoved";
        //only works if the gate is first removed from the module, then added to another (otherwise wrong one is removed)
        auto gate = gNetlist->get_gate_by_id(removed_gate);
        auto gateItem = mGateToTreeitems.value(gate);
        if(!gateItem)
            return;

        beginRemoveRows(parent(getIndexFromItem(gateItem)), gateItem->getOwnRow(), gateItem->getOwnRow());
        mGateToTreeitems.remove(gate);
        gateItem->getParent()->removeChild(gateItem);
        delete gateItem;
        endRemoveRows();
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
        disconnect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
        disconnect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
    }

    void ModuleTreeModel::connectEvents()
    {
        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleTreeModel::handleModuleGateAssigned);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleTreeModel::handleModuleGateRemoved);
    }
}
