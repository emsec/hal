#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "gui/gui_globals.h"

namespace hal
{
    NetlistElementsTreeModel::NetlistElementsTreeModel(QObject *parent) : QAbstractItemModel(parent), mModuleIcon(QIcon(":/icons/sel_module")), mGateIcon(QIcon(":/icons/sel_gate")), mNetIcon(QIcon(":/icons/sel_net"))
    {
        // use root item to store header information
        mRootItem = new TreeItem(QList<QVariant>() << "Name" << "ID" << "Type");
        setModule(gNetlist->get_module_by_id(1));
        //setContent(QList<int>() << 1, QList<int>(), QList<int>());

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

        //for now only standard text data, other roles can follow
        if(role == Qt::DisplayRole)
            return item->getData(index.column());
        if(role == Qt::DecorationRole && index.column() == 0)
            return getIconFromItem(getItemFromIndex(index));

        return QVariant();
    }

    QVariant NetlistElementsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section < mRootItem->getColumnCount())
            return mRootItem->getData(section);
        else
            return QVariant();
    }

    QModelIndex NetlistElementsTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        if(!hasIndex(row, column, parent))
            return QModelIndex();

        TreeItem* parentItem = parent.isValid() ? getItemFromIndex(parent) : mRootItem;
        TreeItem* childItem = parentItem->getChild(row);
        return (childItem) ? createIndex(row, column, childItem) : QModelIndex();
    }

    QModelIndex NetlistElementsTreeModel::parent(const QModelIndex &index) const
    {
        if(!index.isValid())
            return QModelIndex();

        TreeItem* currentItem = getItemFromIndex(index);
        if(!currentItem)
            return QModelIndex();

        TreeItem* parentItem = currentItem->getParent();
        if(parentItem == mRootItem)
            return QModelIndex();

        return getIndexFromItem(parentItem);

    }

    Qt::ItemFlags NetlistElementsTreeModel::flags(const QModelIndex &index) const
    {
        return QAbstractItemModel::flags(index);
    }

int NetlistElementsTreeModel::rowCount(const QModelIndex &parent) const
    {
        if(!parent.isValid()) //no valid parent = root item
            return mRootItem->getChildCount();
        else
            return getItemFromIndex(parent)->getChildCount();
    }

    int NetlistElementsTreeModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mRootItem->getColumnCount();
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

        }
        //no need to check if gates should be displayed, because if not, just give a empty gateIds list (same for nets)
        for(int id : gateIds)
        {
            Gate* gate = gNetlist->get_gate_by_id(id);
            TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                              << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
            gateItem->setAdditionalData(mItemTypeKey, itemType::gate);
            mRootItem->appendChild(gateItem);

        }
        for(int id : netIds)
        {
            Net* net = gNetlist->get_net_by_id(id);
            TreeItem* netItem = new TreeItem(QList<QVariant>() << QString::fromStdString(net->get_name())
                                             << net->get_id() << "");
            netItem->setAdditionalData(mItemTypeKey, itemType::net);
            mRootItem->appendChild(netItem);
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

    QModelIndex NetlistElementsTreeModel::getIndexFromItem(TreeItem *item) const
    {
        assert(item);

        TreeItem* parentItem = item->getParent();

        // if the given item has no parent, it is the root item
        if(!parentItem)
            return createIndex(0,0,mRootItem);

        // get the row of the item and create the modelindex
        int itemRow = parentItem->getRowForChild(item);
        if(itemRow != -1)
            return createIndex(itemRow, 0, item);

        return QModelIndex();
    }

    TreeItem *NetlistElementsTreeModel::getItemFromIndex(QModelIndex index) const
    {
        return (index.isValid()) ? static_cast<TreeItem*>(index.internalPointer()) : nullptr;
    }

    void NetlistElementsTreeModel::clear()
    {
        beginResetModel();
        //delete all children, not the root item
        while(mRootItem->getChildCount() > 0)
        {
            TreeItem* tmp = mRootItem->removeChild(0);
            delete tmp;
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
            subModItem->setAdditionalData(mItemTypeKey, itemType::module);
            modItem->appendChild(subModItem);
        }
        if(showGates)
        {
            for(auto gate : mod->get_gates())
            {
                TreeItem* gateItem = new TreeItem(QList<QVariant>() << QString::fromStdString(gate->get_name())
                                                  << gate->get_id() << QString::fromStdString(gate->get_type()->get_name()));
                gateItem->setAdditionalData(mItemTypeKey, itemType::gate);
                modItem->appendChild(gateItem);

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
