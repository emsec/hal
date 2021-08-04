#include "gui/grouping/grouping_proxy_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    GroupingProxyModel::GroupingProxyModel(QObject* parent) : QSortFilterProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::lexical)
    {
    }

    bool GroupingProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        if(filterRegularExpression().pattern().isEmpty())
            return true;

        QModelIndex source_index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
        if(source_index.isValid())
        {
            if (sourceModel()->data(source_index, filterRole()).toString().contains(filterRegularExpression()))
                return true;
            else
                return false;
        }

        return true;
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
}
