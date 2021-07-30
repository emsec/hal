#include "gui/module_model/module_proxy_model.h"
#include "gui/module_model/module_item.h"

#include "gui/gui_globals.h"

namespace hal
{
    ModuleProxyModel::ModuleProxyModel(QObject* parent) : QSortFilterProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::lexical)
    {
        // QTS PROXY MODELS ARE DUMB, IMPLEMENT CUSTOM SOLUTION OR SWITCH TO A DIFFERENT FILTER METHOD

        // IN VIEW
        // EVERY TIME FILTER CHANGES / ITEM GETS ADDED MODIFY LOCAL DATA STRUCTURE TO REFLECT CURRENT ITEM VISUALS
        // STYLED DELEGATES USE THAT DATA STRUCTURE TO DRAW THEMSELVES
    }

    bool ModuleProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        if(filterRegularExpression().pattern().isEmpty())
            return true;

        QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
        if(sourceIndex.isValid())
        {
            auto item = static_cast<ModuleItem*>(sourceIndex.internalPointer());
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
        return true;
    }

    bool ModuleProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
    {
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
