#include "gui/context_manager_widget/models/context_tree_model.h"
#include "gui/selection_details_widget/selection_details_icon_provider.h"
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
        mContext(nullptr),
        mDirectory(directory)
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
                }
                case 1:
                {
                    return "";
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
                }
                case 1:
                {
                    return mContext->getTimestamp().toString(Qt::SystemLocaleShortDate);
                }
            }
        }
        return QVariant();
    }

    GraphContext *ContextTreeItem::context() const
    {
        return mContext;
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

    ContextTreeModel::ContextTreeModel(QObject* parent) : BaseTreeModel(parent), mCurrentDirectory(nullptr)
    {
        setHeaderLabels(QStringList() << "View Name" << "Timestamp");

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
                if (index.column()) return QVariant(); // decorator only for first column
                if (item->isDirectory())
                    return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ViewDir,0));
                const GraphContext* ctx = item->context();
                u32 mid = 0;
                if (ctx && (mid=ctx->getExclusiveModuleId()) != 0)
                    return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ModuleIcon,mid));
                return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ViewCtx,0));
            }

            case Qt::DisplayRole:
                return item->getData(index.column());

            case Qt::BackgroundRole:
                // must declare color in dark/light style
                return QVariant();

            default:
                return QVariant();
        }

        return QVariant();
    }

    void ContextTreeModel::addDirectory(QString name, BaseTreeItem *parent)
    {
        ContextDirectory* directory = new ContextDirectory(name);

        ContextTreeItem* item   = new ContextTreeItem(directory);

        BaseTreeItem* parentItem = parent;

        if (!parentItem)
            parentItem = mCurrentDirectory;
        if (!parentItem)
            parentItem = mRootItem;


        QModelIndex index = getIndexFromItem(parentItem);

        mCurrentDirectory = item;

        int row = parentItem->getChildCount();
        beginInsertRows(index, row, row);
        parentItem->appendChild(item);
        endInsertRows();

        Q_EMIT directoryCreatedSignal(item);

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
        GraphContext* context = nullptr;
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

    void ContextTreeModel::setCurrentDirectory(ContextTreeItem* currentItem)
    {
        if(currentItem->isContext())
            mCurrentDirectory = (currentItem->getParent() == mRootItem) ? nullptr : static_cast<ContextTreeItem*>(currentItem->getParent());

        else if (currentItem->isDirectory())
            mCurrentDirectory = currentItem;
    }
}
