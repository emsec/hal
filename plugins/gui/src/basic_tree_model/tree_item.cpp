#include "gui/basic_tree_model/tree_item.h"


namespace hal
{
    TreeItem::TreeItem() : mParent(nullptr)
    {

    }

    TreeItem::TreeItem(QList<QVariant> columnData) : mParent(nullptr), mData(columnData)
    {

    }

    TreeItem::~TreeItem()
    {
        for(TreeItem* item : mChildren)
            delete item;
    }

    QVariant TreeItem::getData(int column)
    {
        return (column < 0 || column >= mData.size()) ? QVariant() : mData.at(column);
    }

    void TreeItem::setData(QList<QVariant> data)
    {
        mData = data;
    }

    void TreeItem::setDataAtIndex(int index, QVariant data)
    {
        if(!(index < 0 || index >= mData.size()) && !mData.empty())
            mData.replace(index, data);
    }

    void TreeItem::appendData(QVariant data)
    {
        mData.append(data);
    }

    TreeItem *TreeItem::getParent()
    {
        return mParent;
    }

    void TreeItem::setParent(TreeItem *parent)
    {
        mParent = parent;
    }

    TreeItem *TreeItem::getChild(int row)
    {
        return (row < 0 || row >= mChildren.size()) ? nullptr : mChildren.at(row);
    }

    QList<TreeItem *> TreeItem::getChildren()
    {
        return mChildren;
    }

    void TreeItem::appendChild(TreeItem *child)
    {
        child->setParent(this);
        mChildren.append(child);
    }

    void TreeItem::insertChild(int index, TreeItem *child)
    {
        child->setParent(this);
        mChildren.insert(index, child);
    }

    TreeItem* TreeItem::removeChildAtPos(int row)
    {
        if(row < 0 || row >= mChildren.size())
            return nullptr;
        else
        {
            TreeItem* itemToRemove = mChildren.at(row);
            mChildren.removeAt(row);
            return itemToRemove;
        }
    }

    bool TreeItem::removeChild(TreeItem *child)
    {
        int index = mChildren.indexOf(child);
        if(index == -1)
            return false;
        else
        {
            mChildren.removeAt(index);
            return true;
        }
    }

    int TreeItem::getChildCount()
    {
        return mChildren.size();
    }

    int TreeItem::getColumnCount()
    {
        return mData.size();
    }

    int TreeItem::getRowForChild(TreeItem *child)
    {
        int index = -1;
        for(int i = 0; i < mChildren.size(); i++)
        {
            if(mChildren.at(i) == child)
            {
                index = i;
                break;
            }
        }
        return index;
    }

    int TreeItem::getOwnRow()
    {
        if(!mParent)
            return -1;

        return mParent->getRowForChild(this);
    }

    void TreeItem::setAdditionalData(QString key, QVariant data)
    {
        mAdditionalData.insert(key, data);
    }

    QVariant TreeItem::getAdditionalData(QString key)
    {
        return mAdditionalData.value(key, QVariant());
    }

}
