#include "gui/context_manager_widget/models/context_tree_model.h"
#include "gui/selection_details_widget/selection_details_icon_provider.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/user_action/action_delete_object.h"


#include <limits>
#include <QApplication>


namespace hal
{
    void ContextDirectory::writeToFile(QJsonObject &json)
    {
        json["parentId"] = (int) mParentId;
        json["id"] = (int) mId;
        json["name"] = mName;
    }

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

    u32 ContextTreeItem::getId() const
    {
        if (mDirectory) return mDirectory->id();
        if (mContext) return mContext->id();
        return 0;
    }

    QVariant ContextTreeItem::getData(int column) const
    {
        if(isDirectory())
        {
            switch(column)
            {
                case 0:
                {
                    return mDirectory->name();
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

    ContextDirectory *ContextTreeItem::directory() const
    {
        return mDirectory;
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

    BaseTreeItem* ContextTreeModel::getDirectory(u32 directoryId) const
    {
        return getDirectoryInternal(mRootItem, directoryId);
    }

    BaseTreeItem* ContextTreeModel::getDirectoryInternal(BaseTreeItem *parentItem, u32 directoryId) const
    {
        for (BaseTreeItem* childItem : parentItem->getChildren())
        {
            ContextTreeItem* ctxItem = static_cast<ContextTreeItem*>(childItem);
            if (ctxItem->isDirectory())
            {
                if (ctxItem->getId() == directoryId)
                    return ctxItem;
                BaseTreeItem* bti = getDirectoryInternal(childItem, directoryId);
                if (bti)
                    return bti;
            }
        }
        return nullptr;
    }

    ContextDirectory* ContextTreeModel::addDirectory(QString name, BaseTreeItem *parent, u32 id)
    {

        if(id == 0)
            id = --mMinDirectoryId;
        else if (id < mMinDirectoryId)
            mMinDirectoryId = id;

        BaseTreeItem* parentItem = parent;

        if (!parentItem)
            parentItem = mCurrentDirectory;
        if (!parentItem)
            parentItem = mRootItem;

        ContextDirectory* directory = new ContextDirectory(name, (parentItem != mRootItem) ? static_cast<ContextTreeItem*>(parentItem)->directory()->id() : 0, id);

        ContextTreeItem* item   = new ContextTreeItem(directory);


        QModelIndex index = getIndexFromItem(parentItem);

        mCurrentDirectory = item;

        int row = parentItem->getChildCount();
        beginInsertRows(index, row, row);
        parentItem->appendChild(item);
        endInsertRows();

        mDirectoryList.push_back(directory);

        Q_EMIT directoryCreatedSignal(item);

        return directory;
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

        BaseTreeItem* finalParent;
        if (parent)
            finalParent = parent;
        else if(mCurrentDirectory)
            finalParent = mCurrentDirectory;
        else
            finalParent = mRootItem;


        QModelIndex index = getIndexFromItem(finalParent);

        int row = finalParent->getChildCount();
        beginInsertRows(index, row, row);
        finalParent->appendChild(item);
        endInsertRows();

        mContextMap.insert({context, item});

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

    void ContextTreeModel::removeDirectory(ContextDirectory *directory)
    {
        ContextTreeItem* item   = static_cast<ContextTreeItem *>(getDirectory(directory->id()));

        if(item == mCurrentDirectory) {
            mCurrentDirectory = nullptr;
        }

        QList<BaseTreeItem *> childCopy = item->getChildren();

        for (int i = 0; i < childCopy.length(); i++) {
            ContextTreeItem* child = static_cast<ContextTreeItem*>(childCopy[i]);

            if (child->isContext()) {
                ActionDeleteObject* act = new ActionDeleteObject;
                act->setObject(UserActionObject(child->context()->id(),UserActionObjectType::ContextView));
                act->exec();
            }
            else if (child->isDirectory()) {
                ActionDeleteObject* act = new ActionDeleteObject;
                act->setObject(UserActionObject(child->directory()->id(),UserActionObjectType::ContextDir));
                act->exec();
            }

        }

        BaseTreeItem* parent = item->getParent();
        assert(item);
        assert(parent);

        QModelIndex index = getIndexFromItem(parent);

        int row = item->row();

        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        endRemoveRows();

        auto it = std::find(mDirectoryList.begin(), mDirectoryList.end(), directory);
        if (it != mDirectoryList.end()) {
            mDirectoryList.erase(it);
        }

        delete item;
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

    const QVector<ContextDirectory *> &ContextTreeModel::directoryList()
    {
        return mDirectoryList;
    }


    void ContextTreeModel::setCurrentDirectory(ContextTreeItem* currentItem)
    {
        if(currentItem->isContext())
            mCurrentDirectory = (currentItem->getParent() == mRootItem) ? nullptr : static_cast<ContextTreeItem*>(currentItem->getParent());

        else if (currentItem->isDirectory())
            mCurrentDirectory = currentItem;
    }

}
