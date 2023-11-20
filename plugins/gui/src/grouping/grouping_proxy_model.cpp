



#include "gui/grouping/grouping_proxy_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    GroupingProxyModel::GroupingProxyModel(QObject* parent) : SearchProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::natural)
    {

    }

    bool GroupingProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        return checkRow(sourceRow, sourceParent, 0, 2);
    }

    bool GroupingProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
    {
        // FILTERKEYCOLUMN SHOULD BE EITHER 0 OR 1

        if (filterKeyColumn() == 0)
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
        else
            return source_left.data().toInt() < source_right.data().toInt();

    }

    gui_utility::mSortMechanism GroupingProxyModel::sortMechanism()
    {
        return mSortMechanism;
    }

    void GroupingProxyModel::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
        invalidate();
    }
    void GroupingProxyModel::startSearch(QString text, int options)
    {
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }
}
