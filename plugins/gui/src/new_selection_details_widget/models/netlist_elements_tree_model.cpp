#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "hal_core/netlist/module.h"
#include "gui/gui_globals.h"


namespace hal
{

    NetlistElementsTreeModel::NetlistElementsTreeModel(QObject *parent) : QAbstractItemModel(parent)
    {
        // use root item to store header information
        mRootItem = new TreeItem(QList<QVariant>() << "Name" << "ID" << "Type");
        setModule(gNetlist->get_module_by_id(1));

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

    void NetlistElementsTreeModel::setContent(QList<int> modIds, QList<int> gateIds, QList<int> netIds, bool displayModulesRecursive, bool showGatesInSubmod)
    {
        Q_UNUSED(modIds)
        Q_UNUSED(gateIds)
        Q_UNUSED(netIds)
        Q_UNUSED(displayModulesRecursive)
        Q_UNUSED(showGatesInSubmod)
    }

    void NetlistElementsTreeModel::setModule(Module* mod, bool showGates, bool displayModulesRecursive)
    {
        clear();
        beginResetModel();//also used in clear, but better once too much than once too little
        TreeItem* tmp = new TreeItem(QList<QVariant>() << "tst" << "tst2" << "tst3" << "tst4");
        tmp->setParent(mRootItem);
        mRootItem->appendChild(tmp);
        endResetModel();

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

}
