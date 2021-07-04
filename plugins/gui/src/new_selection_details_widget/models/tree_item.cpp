#include "gui/new_selection_details_widget/models/tree_item.h"


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

    void TreeItem::appendChild(TreeItem *child)
    {
        child->setParent(this);
        mChildren.append(child);
    }

    TreeItem* TreeItem::removeChild(int row)
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

    void TreeItem::setAdditionalData(QString key, QVariant data)
    {
        mAdditionalData.insert(key, data);
    }

    QVariant TreeItem::getAdditionalData(QString key)
    {
        return mAdditionalData.value(key, QVariant());
    }

}
