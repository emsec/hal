#include "gui/basic_tree_model/base_tree_model.h"
#include "gui/basic_tree_model/tree_item.h"

namespace hal
{

    BaseTreeModel::BaseTreeModel(QObject *parent) : QAbstractItemModel(parent)
    {
        mRootItem = new TreeItem();
    }

    QVariant BaseTreeModel::data(const QModelIndex &index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        TreeItem* item = getItemFromIndex(index);
        if(!item)
            return QVariant();

        //The standard model only displays the text
        if(role == Qt::DisplayRole)
            return item->getData(index.column());

        return QVariant();
    }

    QVariant BaseTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section < mRootItem->getColumnCount())
            return mRootItem->getData(section);
        else
            return QVariant();
    }

    QModelIndex BaseTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        if(!hasIndex(row, column, parent))
            return QModelIndex();

        TreeItem* parentItem = parent.isValid() ? getItemFromIndex(parent) : mRootItem;
        TreeItem* childItem = parentItem->getChild(row);
        return (childItem) ? createIndex(row, column, childItem) : QModelIndex();
    }

    QModelIndex BaseTreeModel::parent(const QModelIndex &index) const
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

    Qt::ItemFlags BaseTreeModel::flags(const QModelIndex &index) const
    {
        return QAbstractItemModel::flags(index);
    }

    int BaseTreeModel::rowCount(const QModelIndex &parent) const
    {
        if(!parent.isValid()) //no valid parent = root item
            return mRootItem->getChildCount();
        else
            return getItemFromIndex(parent)->getChildCount();
    }

    int BaseTreeModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mRootItem->getColumnCount();
    }

    void BaseTreeModel::setContent(QList<TreeItem *> firstLevelItems)
    {
        for(auto item : firstLevelItems)
            mRootItem->appendChild(item);
    }

    void BaseTreeModel::clear()
    {
        beginResetModel();
        //delete all children, not the root item
        while(mRootItem->getChildCount() > 0)
        {
            TreeItem* tmp = mRootItem->removeChildAtPos(0);
            delete tmp;
        }
        endResetModel();
    }

    void BaseTreeModel::setHeaderLabels(QList<QVariant> labels)
    {
        if(!mRootItem)
            mRootItem = new TreeItem();

        mRootItem->setData(labels);
    }

    QModelIndex BaseTreeModel::getIndexFromItem(TreeItem *item) const
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

    TreeItem *BaseTreeModel::getItemFromIndex(QModelIndex index) const
    {
        return (index.isValid()) ? static_cast<TreeItem*>(index.internalPointer()) : nullptr;
    }

    TreeItem *BaseTreeModel::getRootItem() const
    {
        return mRootItem;
    }

}
