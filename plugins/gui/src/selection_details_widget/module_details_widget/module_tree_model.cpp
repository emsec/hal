#include "gui/selection_details_widget/module_details_widget/module_tree_model.h"
#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/basic_tree_model/base_tree_item.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"
#include <QQueue>

namespace hal
{


    ModuleTreeitem::ModuleTreeitem(ItemType itp, int id, const QString &name, const QString &ntp)
        : mItemType(itp), mId(id), mName(name), mNodeType(ntp)
    {;}

    QVariant ModuleTreeitem::getData(int index) const
    {
        switch (index)
        {
        case 0:
            return mName;
        case 1:
            return mId;
        case 2:
            return mNodeType;
        }
    }

    void ModuleTreeitem::setData(QList<QVariant> data)
    {
        mName = data[0].toString();
        mId = data[1].toInt();
        mNodeType = data[2].toString();
    }

    void ModuleTreeitem::setDataAtIndex(int index, QVariant &data)
    {
        const char* ctyp[] = { "module", "gate"};

        switch (index)
        {
        case 0: mName = data.toString(); break;
        case 1: mId   = data.toInt(); break;
        case 2:
            for (int j=0; j<3; j++)
                if (data.toString() == ctyp[j])
                {
                    mNodeType = data.toString();
                    break;
                }
        }
    }

    void ModuleTreeitem::appendData(QVariant data)
    {

    }

    int ModuleTreeitem::getColumnCount() const
    {
        return 3;
    }


    ModuleTreeModel::ModuleTreeModel(QObject* parent) : BaseTreeModel(parent), mModId(-1)
    {
        setHeaderLabels(QStringList() << "Name" << "ID" << "Type");
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
            BaseTreeItem* tmp = mRootItem->removeChildAtPos(0);
            delete tmp;
        }

        mModId = m->get_id();
        //add modules
        for(auto mod : m->get_submodules())
        {
            ModuleTreeitem* modItem = new ModuleTreeitem(ModuleTreeitem::Module,
                                                         mod->get_id(),
                                                         QString::fromStdString(mod->get_name()),
                                                         QString::fromStdString(mod->get_type()));
            moduleRecursive(mod, modItem);
            mRootItem->appendChild(modItem);
            mModuleToTreeitems.insert(mod, modItem);
        }
        //add gates
        for(auto gate : m->get_gates())
        {
            ModuleTreeitem* gateItem = new ModuleTreeitem(ModuleTreeitem::Gate,
                                                          gate->get_id(),
                                                          QString::fromStdString(gate->get_name()),
                                                          QString::fromStdString(gate->get_type()->get_name()));
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

        ModuleTreeitem* item = dynamic_cast<ModuleTreeitem*>(getItemFromIndex(index));
        if(!item)
            return QVariant();

        if(role == Qt::DecorationRole && index.column() == 0)
            return getIconFromItem(item);

        //yes, it performs the same two checks again, should be okay though (in terms of performance)
        return BaseTreeModel::data(index, role);

    }

    void ModuleTreeModel::moduleRecursive(Module *mod, BaseTreeItem *modItem)
    {
        ModuleTreeitem* subModItem = nullptr;
        for(Module* subMod : mod->get_submodules())
        {
            subModItem = new ModuleTreeitem(ModuleTreeitem::Module,
                                            subMod->get_id(),
                                            QString::fromStdString(subMod->get_name()),
                                            QString::fromStdString(subMod->get_type()));
            moduleRecursive(subMod, subModItem);
            modItem->appendChild(subModItem);
            mModuleToTreeitems.insert(subMod, subModItem);
        }
        for(auto gate : mod->get_gates())
        {
            ModuleTreeitem* gateItem = new ModuleTreeitem(ModuleTreeitem::Gate,
                                                          gate->get_id(),
                                                          QString::fromStdString(gate->get_name()),
                                                          QString::fromStdString(gate->get_type()->get_name()));
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
        {
            if(static_cast<ModuleTreeitem*>(modItem->getChild(startIndex))->itemType() != ModuleTreeitem::Module)
                break;
        }

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
                ModuleTreeitem* gateItem = new ModuleTreeitem(ModuleTreeitem::Gate,
                                                              gate->get_id(),
                                                              QString::fromStdString(gate->get_name()),
                                                              QString::fromStdString(gate->get_type()->get_name()));
                modItem->appendChild(gateItem);
                mGateToTreeitems.insert(gate, gateItem);
            }
        }
        endResetModel();
    }

    QIcon ModuleTreeModel::getIconFromItem(ModuleTreeitem *item) const
    {
        if(!item)
            return QIcon();

        u32 id = item->getData(1).toInt();
        switch (item->itemType())
        {
        case ModuleTreeitem::Module:
            return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ModuleIcon,id));
        case ModuleTreeitem::Gate:
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
            ModuleTreeitem* addedSubmodItem = new ModuleTreeitem(ModuleTreeitem::Module,
                                                                 addedMod->get_id(),
                                                                 QString::fromStdString(addedMod->get_name()),
                                                                 QString::fromStdString(addedMod->get_type()));
            moduleRecursive(addedMod, addedSubmodItem);
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
        QQueue<BaseTreeItem*> treeItemsQueue;
        treeItemsQueue.enqueue(removedModItem);
        while(!treeItemsQueue.isEmpty())
        {
            ModuleTreeitem* current = static_cast<ModuleTreeitem*>(treeItemsQueue.dequeue());
            switch (current->itemType())
            {
                case ModuleTreeitem::Module: mModuleToTreeitems.remove(gNetlist->get_module_by_id(current->getData(ModuleTreeModel::sIdColumn).toInt())); break;
                case ModuleTreeitem::Gate: mGateToTreeitems.remove(gNetlist->get_gate_by_id(current->getData(ModuleTreeModel::sIdColumn).toInt()));break;
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
        BaseTreeItem* modItem = mModuleToTreeitems.value(m, nullptr);
        if((int)m->get_id() == mModId)
            modItem = mRootItem;

        if(!modItem)
            return;

        auto assignedGate = gNetlist->get_gate_by_id(assigned_gate);
        int indexToInsert = 0; //first item after the modules
        for(; indexToInsert < modItem->getChildCount(); indexToInsert++)
            if(static_cast<ModuleTreeitem*>(modItem->getChild(indexToInsert))->itemType() != ModuleTreeitem::Module)
                break;

        ModuleTreeitem* gateItem = new ModuleTreeitem(ModuleTreeitem::Gate,
                                                      assignedGate->get_id(),
                                                      QString::fromStdString(assignedGate->get_name()),
                                                      QString::fromStdString(assignedGate->get_type()->get_name()));
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
            QVariant qv = QVariant(QString::fromStdString(g->get_name()));
            gateItem->setDataAtIndex(sNameColumn, qv);
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
            QVariant qv = QVariant(QString::fromStdString(m->get_name()));
            moduleItem->setDataAtIndex(sNameColumn, qv);
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
            QVariant qv = QVariant(QString::fromStdString(m->get_type()));
            moduleItem->setDataAtIndex(sTypeColumn, qv);
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
