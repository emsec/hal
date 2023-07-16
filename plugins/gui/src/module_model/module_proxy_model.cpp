#include "gui/module_model/module_proxy_model.h"
#include "gui/module_model/module_item.h"

#include "gui/gui_globals.h"

namespace hal
{
    ModuleProxyModel::ModuleProxyModel(QObject* parent) : QSortFilterProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::lexical), mFilterNets(false), mFilterGates(false)
    {
        // QTS PROXY MODELS ARE DUMB, IMPLEMENT CUSTOM SOLUTION OR SWITCH TO A DIFFERENT FILTER METHOD

        // IN VIEW
        // EVERY TIME FILTER CHANGES / ITEM GETS ADDED MODIFY LOCAL DATA STRUCTURE TO REFLECT CURRENT ITEM VISUALS
        // STYLED DELEGATES USE THAT DATA STRUCTURE TO DRAW THEMSELVES
    }

    bool ModuleProxyModel::toggleFilterNets()
    {
        mFilterNets = !mFilterNets;
        invalidateFilter();
        return mFilterNets;
    }

    bool ModuleProxyModel::toggleFilterGates()
    {
        mFilterGates = !mFilterGates;
        invalidateFilter();
        return mFilterGates;
    }

    bool ModuleProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
        if(!sourceIndex.isValid())
            return true;
        auto item = static_cast<ModuleItem*>(sourceIndex.internalPointer());

        if(mFilterGates && item->getType() == ModuleItem::TreeItemType::Gate)
            return false;
        if(mFilterNets && item->getType() == ModuleItem::TreeItemType::Net)
            return false;

        if(filterRegularExpression().pattern().isEmpty())
            return true;

        if(item->childCount() == 0)
            return sourceModel()->data(sourceIndex, filterRole()).toString().contains(filterRegularExpression());

        bool shouldBeDisplayed = sourceModel()->data(sourceIndex, filterRole()).toString().contains(filterRegularExpression());;
        //go through all children and return the check of itself and the check of the children
        for(int i = 0; i < item->childCount(); i++)
        {
            shouldBeDisplayed = shouldBeDisplayed || filterAcceptsRow(i, sourceIndex);
        }

        return shouldBeDisplayed;
    }

    bool ModuleProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
    {
        ModuleItem* item_left = static_cast<ModuleItem*>(source_left.internalPointer());
        ModuleItem* item_right = static_cast<ModuleItem*>(source_right.internalPointer());
        if(item_left->getType() != item_right->getType())
            return item_left->getType() < item_right->getType();

        QString name_left = source_left.data().toString();
        QString name_right = source_right.data().toString();
        if (sortCaseSensitivity() == Qt::CaseInsensitive)
        {
            name_left = name_left.toLower();
            name_right = name_right.toLower();
        }

        return gui_utility::compare(mSortMechanism, name_left, name_right);
    }

    gui_utility::mSortMechanism ModuleProxyModel::sortMechanism()
    {
        return mSortMechanism;
    }

    void ModuleProxyModel::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
        invalidate();
    }
}
