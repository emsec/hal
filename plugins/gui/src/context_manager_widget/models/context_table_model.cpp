#include "gui/context_manager_widget/models/context_table_model.h"

#include "gui/gui_globals.h"

namespace hal
{

    ContextTreeItem::ContextTreeItem(GraphContext *context) :
        BaseTreeItem(),
        mContext(context)
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

    ContextTableModel::ContextTableModel(QObject* parent) : BaseTreeModel(parent)
    {
        setHeaderLabels(QStringList() << "View Name" << "Timestamp");
    }

    int ContextTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return mContextMap.size();
    }


    QVariant ContextTableModel::data(const QModelIndex& index, int role) const
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

    void ContextTableModel::clear()
    {
        beginResetModel();

        BaseTreeModel::clear();
        mContextMap.clear();

        endResetModel();
    }

    void ContextTableModel::addContext(GraphContext* context, BaseTreeItem* parent)
    {
        ContextTreeItem* item   = new ContextTreeItem(context);

        item->setParent(parent);

        QModelIndex index = getIndex(parent);

        int row = parent->getChildCount();
        beginInsertRows(index, row, row);
        parent->appendChild(item);
        endInsertRows();

        mContextMap.insert({context, item});

        //connect(context,&GraphContext::dataChanged,this,&ContextTableModel::handleDataChanged);
        connect(context, &GraphContext::dataChanged, this, [item, this]() {
            Q_EMIT dataChanged(getIndex(item), getIndex(item));
        });
    }

    void ContextTableModel::removeContext(GraphContext *context)
    {
        ContextTreeItem* item   = mContextMap.find(context)->second;
        ContextTreeItem* parent = static_cast<ContextTreeItem*>(item->getParent());
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        endRemoveRows();

        delete item;

        std::map<GraphContext *,ContextTreeItem *>::iterator it;
        it = mContextMap.find (context);
        mContextMap.erase (it);
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
    }

    GraphContext* ContextTableModel::getContext(const QModelIndex& index) const
    {
        return (mContextList)[index.row()];
    }*/

    QModelIndex ContextTableModel::getIndex(const BaseTreeItem* const item) const
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
    }

    const QVector<GraphContext *> &ContextTableModel::list() const
    {
        QVector<GraphContext *> key;
        for (auto it = mContextMap.begin(); it != mContextMap.end(); ++it) {
          key.push_back(it->first);
        }
        return key;
    }
}
