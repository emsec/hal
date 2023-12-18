#include "gui/context_manager_widget/models/context_table_model.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

#include <limits>
#include <QApplication>


namespace hal
{

    ContextTreeItem::ContextTreeItem(GraphContext *context) :
        BaseTreeItem(),
        mContext(context),
        mDirectory(nullptr)
    {
    }

    ContextTreeItem::ContextTreeItem(ContextDirectory *directory) :
        BaseTreeItem(),
        mDirectory(directory),
        mContext(nullptr)
    {
    }

    QVariant ContextTreeItem::getData(int column) const
    {
        if(isDirectory())
        {
            switch(column)
            {
                case 0:
                {
                    return mDirectory->getName();
                    break;
                }
                case 1:
                {
                    return "";
                    break;
                }
            }
        }
        if(isContext())
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
        //connect(qApp, &QApplication::focusChanged, this, &ContextTreeModel::itemFocusChanged);

    }

    QVariant ContextTreeModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        ContextTreeItem* item = static_cast<ContextTreeItem*>(index.internalPointer());

        if (!item)
            return QVariant();

        switch (role)
        {
            case Qt::DecorationRole:
            {
                break;
            }
            case Qt::DisplayRole:
            {
                switch(index.column())
                {
                    case 0: return item->getData(0); break;
                    case 1: return item->getData(1); break;
                    default: return QVariant();
                }
                break;
            }
            case Qt::BackgroundRole:
            {
                if (item->isDirectory())
                    return QColor(QColor( 32, 32, 32));
                break;
            }
            default:
                return QVariant();
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

        mCurrentDirectory = item;

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

    GraphContext* ContextTreeModel::getContext(const QModelIndex& index) const
    {
        BaseTreeItem* item = getItemFromIndex(index);

        if (static_cast<ContextTreeItem*>(item)->isDirectory()) return nullptr;
        GraphContext* context;
        for (auto &i : mContextMap) {
           if (i.second == item) {
              context = i.first;
              break;
           }
        }
        return context;
    }

    const QVector<GraphContext *> &ContextTreeModel::list()
    {
        mContextList.clear();
        for (auto it = mContextMap.begin(); it != mContextMap.end(); ++it) {
          mContextList.push_back(it->first);
        }
        return mContextList;
    }

    void ContextTreeModel::itemFocusChanged(const QModelIndex &newIndex)
    {
        BaseTreeItem* currentItem = getItemFromIndex(newIndex);
        if(currentItem != mRootItem && static_cast<ContextTreeItem*>(currentItem)->isDirectory())
            mCurrentDirectory = static_cast<ContextTreeItem*>(currentItem);
    }
}
