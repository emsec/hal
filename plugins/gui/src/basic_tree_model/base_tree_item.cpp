#include "gui/basic_tree_model/base_tree_item.h"
#include <iostream>

namespace hal
{
    BaseTreeItem::BaseTreeItem() : mParent(nullptr)
    {

    }


    BaseTreeItem::~BaseTreeItem()
    {
        for(BaseTreeItem* item : mChildren)
            delete item;
    }


    BaseTreeItem *BaseTreeItem::getParent() const
    {
        return mParent;
    }

    void BaseTreeItem::setParent(BaseTreeItem *parent)
    {
        mParent = parent;
    }

    BaseTreeItem *BaseTreeItem::getChild(int row) const
    {
        return (row < 0 || row >= mChildren.size()) ? nullptr : mChildren.at(row);
    }

    QList<BaseTreeItem *> BaseTreeItem::getChildren() const
    {
        return mChildren;
    }

    void BaseTreeItem::appendChild(BaseTreeItem *child)
    {
        child->setParent(this);
        mChildren.append(child);
    }

    void BaseTreeItem::insertChild(int index, BaseTreeItem *child)
    {
        child->setParent(this);
        mChildren.insert(index, child);
    }

    BaseTreeItem* BaseTreeItem::removeChildAtPos(int row)
    {
        if(row < 0 || row >= mChildren.size())
            return nullptr;
        else
        {
            BaseTreeItem* itemToRemove = mChildren.at(row);
            mChildren.removeAt(row);
            return itemToRemove;
        }
    }

    bool BaseTreeItem::removeChild(BaseTreeItem *child)
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

    int BaseTreeItem::getChildCount() const
    {
        return mChildren.size();
    }

    int BaseTreeItem::getRowForChild(const BaseTreeItem *child) const
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

    int BaseTreeItem::getOwnRow()
    {
        if(!mParent)
            return -1;

        return mParent->getRowForChild(this);
    }

    QVariant RootTreeItem::getData(int column) const
        {
            if (column <= mHeaderLabels.size())
                return mHeaderLabels.at(column);
            return QVariant();
        }

    void RootTreeItem::setData(QList<QVariant> data)
    {
        mHeaderLabels.clear();
        for (int i = 0; i < data.length(); i++) {
            mHeaderLabels[i] = data[i].toString();
        }
    }

    void RootTreeItem::setDataAtColumn(int column, QVariant &data)
    {
        while (column >= mHeaderLabels.size())
            mHeaderLabels << QString();
        mHeaderLabels[column] = data.toString();
    }

    void RootTreeItem::appendData(QVariant data)
    {

    }

}
