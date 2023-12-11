#include "gui/context_manager_widget/models/context_table_model.h"

#include "gui/gui_globals.h"

#include <limits>

namespace hal
{

    ContextTreeItem::ContextTreeItem(GraphContext *context) :
        BaseTreeItem(),
        mContext(context)
    {
    }

    ContextTreeItem::ContextTreeItem(ContextDirectory *directory) :
        BaseTreeItem(),
        mDirectory(directory)
    {
    }

    QVariant ContextTreeItem::getData(int column) const
    {
        switch(column)
        {
            case 0:
            {
                return mContext->getNameWithDirtyState();
                break;
            }
            case 1:
            {
                return mContext->getTimestamp().toString(Qt::SystemLocaleShortDate);
                break;
            }
        }
    }

    void ContextTreeItem::setData(QList<QVariant> data)
    {

    }

    void ContextTreeItem::setDataAtIndex(int index, QVariant &data)
    {

    }

    void ContextTreeItem::appendData(QVariant data)
    {

    }

    int ContextTreeItem::getColumnCount() const
    {
        return 2;
    }

    int ContextTreeItem::row() const
    {
        BaseTreeItem* parent = getParent();
        if (!parent) return 0;
        return parent->getRowForChild(this);
    }

    bool ContextTreeItem::isDirectory() const
    {
        return mDirectory != nullptr;
    }

    bool ContextTreeItem::isContext() const
    {
        return mContext != nullptr;
    }

    ContextTreeModel::ContextTreeModel(QObject* parent) : BaseTreeModel(parent), mCurrentDirectory(nullptr), mMinDirectoryId(std::numeric_limits<u32>::max())
    {
        setHeaderLabels(QStringList() << "View Name" << "Timestamp");
    }

    /*int ContextTreeModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return mContextMap.size();
    }*/


    QVariant ContextTreeModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        ContextTreeItem* item = static_cast<ContextTreeItem*>(index.internalPointer());

        if (!item)
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            switch(index.column())
            {
                case 0: return item->getData(0); break;
                case 1: return item->getData(1); break;
                default: return QVariant();
            }
        }

        return QVariant();
    }

    void ContextTreeModel::addDirectory(QString name, BaseTreeItem *parent)
    {
        ContextDirectory* directory = new ContextDirectory(--mMinDirectoryId, name);

        ContextTreeItem* item   = new ContextTreeItem(directory);

        if (parent)
            item->setParent(parent);
        else if(mCurrentDirectory)
            item->setParent(mCurrentDirectory);
        else
            item->setParent(mRootItem);


        QModelIndex index = getIndexFromItem(item->getParent());

        int row = item->getParent()->getChildCount();
        beginInsertRows(index, row, row);
        item->getParent()->appendChild(item);
        endInsertRows();


        //connect(context,&GraphContext::dataChanged,this,&ContextTableModel::handleDataChanged);
        /*connect(context, &GraphContext::dataChanged, this, [item, this]() {
            Q_EMIT dataChanged(getIndexFromItem(item), getIndexFromItem(item));
        });*/
    }

    void ContextTreeModel::clear()
    {
        beginResetModel();

        BaseTreeModel::clear();
        mContextMap.clear();

        endResetModel();
    }

    void ContextTreeModel::addContext(GraphContext* context, BaseTreeItem* parent)
    {
        ContextTreeItem* item   = new ContextTreeItem(context);

        if (parent)
            item->setParent(parent);
        else if(mCurrentDirectory)
            item->setParent(mCurrentDirectory);
        else
            item->setParent(mRootItem);


        QModelIndex index = getIndexFromItem(item->getParent());

        int row = item->getParent()->getChildCount();
        beginInsertRows(index, row, row);
        item->getParent()->appendChild(item);
        endInsertRows();

        mContextMap.insert({context, item});

        //connect(context,&GraphContext::dataChanged,this,&ContextTableModel::handleDataChanged);
        connect(context, &GraphContext::dataChanged, this, [item, this]() {
            Q_EMIT dataChanged(getIndexFromItem(item), getIndexFromItem(item));
        });
    }

    void ContextTreeModel::removeContext(GraphContext *context)
    {
        ContextTreeItem* item   = mContextMap.find(context)->second;
        ContextTreeItem* parent = static_cast<ContextTreeItem*>(item->getParent());
        assert(item);
        assert(parent);

        QModelIndex index = getIndexFromItem(parent);

        int row = item->row();

        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        endRemoveRows();

        delete item;

        std::map<GraphContext *,ContextTreeItem *>::iterator it;
        it = mContextMap.find(context);
        mContextMap.erase(it);
    }

    QModelIndex ContextTreeModel::getIndexFromContext(GraphContext *context) const
    {
        return getIndexFromItem(mContextMap.find(context)->second);
    }

    /*void ContextTableModel::handleDataChanged()
    {
        GraphContext* gc = static_cast<GraphContext*>(sender());
        if (!gc) return;
        for (int irow = 0; irow < mContextList.size(); ++irow)
        {
            if (gc == mContextList.at(irow))
            {
                QModelIndex inx0 = index(irow,0);
                QModelIndex inx1 = index(irow,1);
                Q_EMIT dataChanged(inx0,inx1);
                return;
            }
        }
    }*/

    GraphContext* ContextTreeModel::getContext(const QModelIndex& index) const
    {
        BaseTreeItem* item = getItemFromIndex(index);

        GraphContext* context;
        for (auto &i : mContextMap) {
           if (i.second == item) {
              context = i.first;
              break;
           }
        }
        return context;
    }

   /* QModelIndex ContextTableModel::getIndex(const BaseTreeItem* const item) const
    {
        assert(item);

        QVector<int> row_numbers;
        const BaseTreeItem* current_item = item;

        while (current_item != mRootItem)
        {
            row_numbers.append(static_cast<const ContextTreeItem*>(current_item)->row());
            current_item = current_item->getParent();
        }

        QModelIndex model_index = index(0, 0, QModelIndex());

        for (QVector<int>::const_reverse_iterator i = row_numbers.crbegin(); i != row_numbers.crend(); ++i)
            model_index = index(*i, 0, model_index);

        return model_index;
    }*/

    const QVector<GraphContext *> &ContextTreeModel::list()
    {
        mContextList.clear();
        for (auto it = mContextMap.begin(); it != mContextMap.end(); ++it) {
          mContextList.push_back(it->first);
        }
        return mContextList;
    }
}
