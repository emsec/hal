#include "gui/basic_tree_model/base_tree_model.h"
#include "gui/basic_tree_model/base_tree_item.h"

namespace hal
{

    BaseTreeModel::BaseTreeModel(QObject *parent) : QAbstractItemModel(parent)
    {
        mRootItem = new RootTreeItem(QStringList());
    }

    QVariant BaseTreeModel::data(const QModelIndex &index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        BaseTreeItem* item = getItemFromIndex(index);
        if(!item)
            return QVariant();

        //The standard model only displays the text
        if(role == Qt::DisplayRole)
            return item->getData(index.column());

        return QVariant();
    }

    QVariant BaseTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section < mRootItem->getColumnCount() && section >= 0)
            return mRootItem->getData(section);
        else
            return QVariant();
    }

    QModelIndex BaseTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        if(!hasIndex(row, column, parent))
            return QModelIndex();

        BaseTreeItem* parentItem = parent.isValid() ? getItemFromIndex(parent) : mRootItem;
        BaseTreeItem* childItem = parentItem->getChild(row);
        return (childItem) ? createIndex(row, column, childItem) : QModelIndex();
    }

    QModelIndex BaseTreeModel::parent(const QModelIndex &index) const
    {
        if(!index.isValid())
            return QModelIndex();

        BaseTreeItem* currentItem = getItemFromIndex(index);
        if(!currentItem)
            return QModelIndex();

        BaseTreeItem* parentItem = currentItem->getParent();
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
        if(!parent.isValid()) // no valid parent = root item
            return mRootItem->getChildCount();
        else if (parent.column()) // only first column is allowed to have children
            return 0;
        else
            return getItemFromIndex(parent)->getChildCount();
    }

    int BaseTreeModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mRootItem->getColumnCount();
    }

    void BaseTreeModel::setContent(QList<BaseTreeItem *> firstLevelItems)
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
            BaseTreeItem* tmp = mRootItem->removeChildAtPos(0);
            delete tmp;
        }
        endResetModel();
    }

    QStringList BaseTreeModel::headerLabels() const
    {
        QStringList retval;
        int ncol = columnCount();
        for (int icol = 0; icol < ncol; icol++)
            retval.append(headerData(icol,Qt::Horizontal,Qt::DisplayRole).toString());
        return retval;
    }

    void BaseTreeModel::setHeaderLabels(const QStringList& labels)
    {
        if(!mRootItem)
            mRootItem = new RootTreeItem(labels);
        else
            for (int i=0; i<labels.size(); i++) {
                QVariant qv = QVariant(labels.at(i));
                mRootItem->setDataAtColumn(i, qv);
            }


    }

    QModelIndex BaseTreeModel::getIndexFromItem(BaseTreeItem *item) const
    {
        assert(item);

        BaseTreeItem* parentItem = item->getParent();

        // if the given item has no parent, it is the root item
        if(!parentItem)
            return QModelIndex();

        // get the row of the item and create the modelindex
        int itemRow = parentItem->getRowForChild(item);
        if(itemRow != -1)
            return createIndex(itemRow, 0, item);

        return QModelIndex();
    }

    BaseTreeItem *BaseTreeModel::getItemFromIndex(QModelIndex index) const
    {
        return (index.isValid()) ? static_cast<BaseTreeItem*>(index.internalPointer()) : nullptr;
    }

    BaseTreeItem *BaseTreeModel::getRootItem() const
    {
        return mRootItem;
    }

    void BaseTreeModel::insertChildItem(BaseTreeItem* childItem, BaseTreeItem* parentItem, int row)
    {
        if (!parentItem) parentItem = mRootItem;
        if (row < 0) row = parentItem->getChildCount();
        QModelIndex index = getIndexFromItem(parentItem);
        beginInsertRows(index, row, row);
        parentItem->insertChild(row,childItem);
        endInsertRows();
    }

}
