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

    QModelIndex ModuleModel::getIndex(const ModuleItem* const item) const
    {
        assert(item);

        QVector<int> row_numbers;
        const ModuleItem* current_item = item;

        while (current_item != mRootItem->getChild(0))
        {
            row_numbers.append(current_item->row());
            current_item = static_cast<const ModuleItem*>(current_item->getParent());
        }

        QModelIndex model_index = index(0, 0, QModelIndex());

        for (QVector<int>::const_reverse_iterator i = row_numbers.crbegin(); i != row_numbers.crend(); ++i)
            model_index = index(*i, 0, model_index);

        return model_index;
    }

    void ModuleModel::init()
    {
        ModuleItem* item = new ModuleItem(1);
        mModuleMap.insert(1, item);

        beginInsertRows(index(0, 0, QModelIndex()), 0, 0);
        mRootItem->appendChild(item);
        endInsertRows();

        Module* m = gNetlist->get_top_module();
        addRecursively(m);
        for(auto net : gNetlist->get_top_module()->get_internal_nets())
        {
            addNet(net->get_id(), m->get_id());
            updateNet(net);
        }
    }

    void ModuleModel::clear()
    {
        beginResetModel();

        BaseTreeModel::clear();
        mModuleMap.clear();
        mGateMap.clear();
        mNetMap.clear();
        //TODO : clear colors
        endResetModel();
    }

    void ModuleModel::addModule(u32 id, u32 parent_module)
    {
        assert(gNetlist->get_module_by_id(id));
        assert(gNetlist->get_module_by_id(parent_module));
        assert(!mModuleMap.contains(id));
        assert(mModuleMap.contains(parent_module));

        ModuleItem* item   = new ModuleItem(id);
        ModuleItem* parent = mModuleMap.value(parent_module);

        item->setParent(parent);
        mModuleMap.insert(id, item);

        QModelIndex index = getIndex(parent);

        int row = parent->getChildCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parent->appendChild(item);
        mIsModifying = false;
        endInsertRows();
    }

    void ModuleModel::addGate(u32 id, u32 parent_module)
    {
        assert(gNetlist->get_gate_by_id(id));
        assert(gNetlist->get_module_by_id(parent_module));
        assert(!mGateMap.contains(id));
        assert(mModuleMap.contains(parent_module));

        ModuleItem* item   = new ModuleItem(id, ModuleItem::TreeItemType::Gate);
        ModuleItem* parent = mModuleMap.value(parent_module);

        item->setParent(parent);
        mGateMap.insert(id, item);

        QModelIndex index = getIndex(parent);

        int row = parent->getChildCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parent->appendChild(item);
        mIsModifying = false;
        endInsertRows();
    }

    void ModuleModel::addNet(u32 id, u32 parent_module)
    {
        assert(gNetlist->get_net_by_id(id));
        assert(gNetlist->get_module_by_id(parent_module));
        assert(!mNetMap.contains(id));
        assert(mModuleMap.contains(parent_module));

        ModuleItem* item   = new ModuleItem(id, ModuleItem::TreeItemType::Net);
        ModuleItem* parent = mModuleMap.value(parent_module);

        item->setParent(parent);
        mNetMap.insert(id, item);

        QModelIndex index = getIndex(parent);

        int row = parent->getChildCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parent->appendChild(item);
        mIsModifying = false;
        endInsertRows();
    }

    void ModuleModel::addRecursively(const Module* module)
    {
        if(!module->is_top_module())
            addModule(module->get_id(), module->get_parent_module()->get_id());
        for(auto &m : module->get_submodules())
            addRecursively(m);

        for(auto &g : module->get_gates())
            addGate(g->get_id(), module->get_id());
    }

    void ModuleModel::removeModule(const u32 id)
    {
        assert(id != 1);
        // module was most likely already purged from netlist
        assert(mModuleMap.contains(id));

        ModuleItem* item   = mModuleMap.value(id);
        ModuleItem* parent = static_cast<ModuleItem*>(item->getParent());
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        mIsModifying = false;
        endRemoveRows();

        mModuleMap.remove(id);
        delete item;
    }

    void ModuleModel::removeGate(const u32 id)
    {
        //assert(gNetlist->get_gate_by_id(id));
        assert(mGateMap.contains(id));

        ModuleItem* item   = mGateMap.value(id);
        ModuleItem* parent = static_cast<ModuleItem*>(item->getParent());
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        mIsModifying = false;
        endRemoveRows();

        mGateMap.remove(id);
        delete item;
    }

    void ModuleModel::removeNet(const u32 id)
    {
        //assert(gNetlist->get_net_by_id(id));
        if(!mNetMap.contains(id)) // global nets are not contained in the item model
            return;

        ModuleItem* item   = mNetMap.value(id);
        ModuleItem* parent = static_cast<ModuleItem*>(item->getParent());
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        mIsModifying = false;
        endRemoveRows();

        mNetMap.remove(id);
        delete item;
    }

    void ModuleModel::handleModuleParentChanged(const Module* module)
    {
        assert(module);
        updateModuleParent(module);

        for(Net* net : module->get_nets())
            updateNet(net);
    }

    void ModuleModel::handleModuleGateAssinged(const u32 id, const u32 parent_module)
    {
        // Don't need new function handleModuleGateRemoved(), because the GateAssinged event always follows GateRemoved
        // or NetlistInternalManager updates Net connections when a gate is deleted.

        if(!mGateMap.contains(id))
            addGate(id, parent_module);
            
        Gate* gate = gNetlist->get_gate_by_id(id);
        for(Net* in_net : gate->get_fan_in_nets())
            updateNet(in_net);
        for(Net* in_net : gate->get_fan_out_nets())
            updateNet(in_net);
    }

    void ModuleModel::updateNet(const Net* net)
    {
        assert(net);
        u32 id = net->get_id();

        if(!mNetMap.contains(id))
            return;
            
        ModuleItem* item = mNetMap.value(id);
        ModuleItem* oldParentItem = static_cast<ModuleItem*>(item->getParent());
        assert(oldParentItem);

        Module* newParentModule = findNetParent(net);
        if(newParentModule == nullptr)
            newParentModule = gNetlist->get_top_module();
        if(newParentModule->get_id() == oldParentItem->id())
            return;

        assert(mModuleMap.contains(newParentModule->get_id()));
        ModuleItem* newParentItem = mModuleMap[newParentModule->get_id()];
        QModelIndex newIndex = getIndex(newParentItem);
        QModelIndex oldIndex = getIndex(oldParentItem);
        int row = item->row();

        mIsModifying = true;
        beginMoveRows(oldIndex, row, row, newIndex, newParentItem->getChildCount());
        oldParentItem->removeChild(item);
        newParentItem->appendChild(item);
        mIsModifying = false;
        endMoveRows();
    }

    void ModuleModel::updateModuleParent(const Module* module){
        assert(module);
        u32 id = module->get_id();
        assert(id != 1);
        assert(mModuleMap.contains(id));
        ModuleItem* item = mModuleMap.value(id);
        ModuleItem* oldParent = static_cast<ModuleItem*>(item->getParent());
        assert(oldParent);

        assert(module->get_parent_module());
        if(oldParent->id() == module->get_parent_module()->get_id())
            return;

        assert(mModuleMap.contains(module->get_parent_module()->get_id()));
        ModuleItem* newParent = mModuleMap.value(module->get_parent_module()->get_id());

        QModelIndex oldIndex = getIndex(oldParent);
        QModelIndex newIndex = getIndex(newParent);
        int row = item->row();

        mIsModifying = true;
        beginMoveRows(oldIndex, row, row, newIndex, newParent->getChildCount());
        oldParent->removeChild(item);
        newParent->appendChild(item);
        mIsModifying = false;
        endMoveRows();
    }

    void ModuleModel::updateModuleName(u32 id)
    {
        assert(gNetlist->get_module_by_id(id));
        assert(mModuleMap.contains(id));

        ModuleItem* item = mModuleMap.value(id);
        assert(item);

        item->setName(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

        QModelIndex index = getIndex(item);
        Q_EMIT dataChanged(index, index);
    }

    void ModuleModel::updateGateName(u32 id)
    {
        assert(gNetlist->get_gate_by_id(id));
        assert(mGateMap.contains(id));

        ModuleItem* item = mGateMap.value(id);
        assert(item);

        item->setName(QString::fromStdString(gNetlist->get_gate_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

        QModelIndex index = getIndex(item);
        Q_EMIT dataChanged(index, index);
    }

    void ModuleModel::updateNetName(u32 id)
    {
        assert(gNetlist->get_net_by_id(id));
        assert(mNetMap.contains(id));

        ModuleItem* item = mNetMap.value(id);
        assert(item);
        
        item->setName(QString::fromStdString(gNetlist->get_net_by_id(id)->get_name()));    // REMOVE & ADD AGAIN

        QModelIndex index = getIndex(item);
        Q_EMIT dataChanged(index, index);
    }

    ModuleItem* ModuleModel::getItem(u32 id, ModuleItem::TreeItemType type) const
    {
        return mModuleItemMaps[(int)type]->value(id);
    }

    bool ModuleModel::isModifying()
    {
        return mIsModifying;
    }
    
    Module* ModuleModel::findNetParent(const Net* net){
        // cannot use Module::get_internal_nets(), because currently that function is implemented so, 
        // that a net can be "internal" to multiple modules at the same depth.
        // => instead manually search for deepest module, that contains all sources and destinations of net.
        assert(net);
        if(net->get_num_of_sources() == 0 && net->get_num_of_destinations() == 0)
            return nullptr;

        std::vector<Endpoint*> endpoints = net->get_sources();

        {
            std::vector<Endpoint*> destinations = net->get_destinations();
            endpoints.insert(endpoints.end(), destinations.begin(), destinations.end());
        }

        Module* parent = endpoints[0]->get_gate()->get_module();
        endpoints.erase(endpoints.begin());

        // might want to split up endpoints, if sources and destinations should be handled differently
        while(endpoints.size() > 0)
        {
            std::vector<Endpoint*>::iterator it = endpoints.begin();
            while(it != endpoints.end())
            {
                if(parent->contains_gate((*it)->get_gate(), true))
                    it = endpoints.erase(it);
                else
                    ++it;
            }

            if(endpoints.size() > 0)
                parent = parent->get_parent_module();
        }

        return parent;
    }
}
