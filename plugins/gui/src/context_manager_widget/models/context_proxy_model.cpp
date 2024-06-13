#include "gui/context_manager_widget/models/context_proxy_model.h"

#include "gui/gui_utils/sort.h"
#include "gui/basic_tree_model/base_tree_model.h"
#include "gui/context_manager_widget/models/context_tree_model.h"
#include "gui/user_action/action_move_item.h"

#include <QDateTime>

namespace hal
{
    ContextProxyModel::ContextProxyModel(QObject* parent) : SearchProxyModel(parent)
    {

    }

    bool ContextProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        return checkRowRecursion(source_row, source_parent, 0, 1);
    }

    bool ContextProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        BaseTreeModel* model = static_cast<BaseTreeModel*>(sourceModel());
        if (!model) return false;

        ContextTreeItem* ctiLeft = dynamic_cast<ContextTreeItem*>(model->getItemFromIndex(left));
        ContextTreeItem* ctiRight = dynamic_cast<ContextTreeItem*>(model->getItemFromIndex(right));

        // root item first
        if (!ctiRight) return false;
        if (!ctiLeft) return true;

        // directory before view
        if (ctiRight->isDirectory() && !ctiLeft->isDirectory()) return false;
        if (ctiLeft->isDirectory() && !ctiRight->isDirectory()) return false;

        switch (left.column())
        {
        case 0:
            return gui_utility::compare(gui_utility::mSortMechanism::numerated, ctiLeft->getName(), ctiRight->getName());
        case 1:
            return ctiLeft->getId() < ctiRight->getId();
        case 2:
            return ctiLeft->getTimestamp() < ctiRight->getTimestamp();
        default:
            break;
        }

        return false;
    }

    void ContextProxyModel::startSearch(QString text, int options)
    {
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }

    QStringList ContextProxyModel::mimeTypes() const
    {
        return sourceModel()->mimeTypes();
    }

    QMimeData* ContextProxyModel::mimeData(const QModelIndexList &indexes) const
    {
        QMimeData* retval = new QMimeData;
        // only single row allowed
        int row = -1;
        for (const QModelIndex& inx : indexes)
        {
            if (row < 0)
                row = inx.row();
            else if (row != inx.row())
                return retval;
        }
        if (row < 0)
            return retval;

        BaseTreeModel* model = static_cast<BaseTreeModel*>(sourceModel());
        if (!model) return retval;

        QModelIndex sourceIndex = mapToSource(indexes.at(0));
        BaseTreeItem* bti = model->getItemFromIndex(sourceIndex);
        row = sourceIndex.row();
        BaseTreeItem* parentItem = bti->getParent();
        ContextTreeItem* item = dynamic_cast<ContextTreeItem*>(bti);
        if (!item)
        {
            qDebug() << "cannot cast" << indexes.at(0);
            return retval;
        }
        QByteArray encodedData;
        QDataStream stream(&encodedData, QIODevice::WriteOnly);
        QString moveType;
        int id;
        if (item->isDirectory())
        {
            id = item->directory()->id();
            moveType = "dir";
        }
        else if (item->isContext())
        {
            id = item->context()->id();
            moveType = "view";
        }
        else
            Q_ASSERT (1==0);
        stream << moveType << id << row << (quintptr) parentItem;
        retval->setText(moveType);
        retval->setData("contexttreemodel/item", encodedData);
        return retval;

    }

    bool ContextProxyModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        Q_UNUSED(column);
        Q_UNUSED(action);

        BaseTreeModel* model = static_cast<BaseTreeModel*>(sourceModel());
        if (!model) return false;

        QString moveType;
        int moveId;
        int sourceRow = -1;
        quintptr sourceParent = 0;

        auto encItem = mimeData->data("contexttreemodel/item");
        QDataStream dataStream(&encItem, QIODevice::ReadOnly);
        dataStream >> moveType >> moveId >> sourceRow >> sourceParent;
        ContextTreeItem* sourceParentItem = dynamic_cast<ContextTreeItem*>((BaseTreeItem*) sourceParent);
        u32 sourceParentId = sourceParentItem ? sourceParentItem->getId() : 0;

        QModelIndex moveInx = model->index(sourceRow, 0, model->getIndexFromItem((BaseTreeItem*) sourceParent)); // source model index to item
        ContextTreeItem* itemToMove = dynamic_cast<ContextTreeItem*>(model->getItemFromIndex(moveInx));
        UserActionObject uao;
        if (itemToMove->isContext())
            uao = UserActionObject(itemToMove->context()->id(), UserActionObjectType::ContextView);
        else if (itemToMove->isDirectory())
            uao = UserActionObject(itemToMove->directory()->id(), UserActionObjectType::ContextDir);
        else
            return false;

        u32 targetParentId = 0;

        if (parent.isValid())
        {
            BaseTreeItem* targetParentItem = model->getItemFromIndex(mapToSource(parent));
            ContextTreeItem* cti = dynamic_cast<ContextTreeItem*>(targetParentItem);
            if (cti)
            {
                if (!cti->isDirectory()) return false;
                targetParentId = cti->directory()->id();
            }

        }

        if (sourceParentId == targetParentId)
        {
            if (sourceRow == row || sourceRow == row-1) return false; // nothing to do
            if (sourceRow < row) row -= 1; // move up, take removed source item into account
        }

        ActionMoveItem* act = new ActionMoveItem(targetParentId, sourceParentId, row);
        act->setObject(uao);
        act->exec();

        return true;
    }

    bool ContextProxyModel::canDropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
    {
        Q_UNUSED(column)
        Q_UNUSED(action)
        Q_UNUSED(row)

        BaseTreeModel* model = static_cast<BaseTreeModel*>(sourceModel());
        if (!model) return false;

        if (!parent.isValid())
            return true; // can always drop on root

        int moveRow = -1;
        quintptr moveParent = 0;
        if(!mimeData->formats().contains("contexttreemodel/item")) return false;

/*
        {
            if (parent.isValid())
            {
                ContextTreeItem* cti = dynamic_cast<ContextTreeItem*>(getItemFromIndex(parent));
                if (cti)
                    qDebug() << "drop on " << (cti->isDirectory() ? "directory" : "view") << cti->getId() << row << column;
                else
                    qDebug() << "drop on root " << row << column;
            }
            else
                qDebug() << "drop no parent " << row << column;
        }
        */
        BaseTreeItem* tpar = model->getItemFromIndex(mapToSource(parent));
        if (tpar == model->getRootItem()) return true;
        ContextTreeItem* targetParentItem = dynamic_cast<ContextTreeItem*>(tpar);
        if (!targetParentItem || targetParentItem->isContext()) return false;

        QString moveType;
        int moveId;
        auto encItem = mimeData->data("contexttreemodel/item");
        QDataStream dataStream(&encItem, QIODevice::ReadOnly);
        dataStream >> moveType >> moveId >> moveRow >> moveParent;

        if (moveType == "dir")
        {
            // target must not be the item itself or any (grand-)child of the item
            ContextTreeItem* targetAnchestorItem = targetParentItem;
            while (targetAnchestorItem)
            {
                if (targetAnchestorItem->isDirectory() && (int) targetAnchestorItem->directory()->id() == moveId)
                    return false;
                targetAnchestorItem = dynamic_cast<ContextTreeItem*>(targetAnchestorItem->getParent());
            }
        }

        return true;
    }

}
