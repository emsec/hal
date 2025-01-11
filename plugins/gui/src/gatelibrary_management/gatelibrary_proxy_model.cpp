#include "gui/gatelibrary_management/gatelibrary_proxy_model.h"
#include "gui/searchbar/search_proxy_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    GatelibraryProxyModel::GatelibraryProxyModel(QObject* parent) : SearchProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::lexical)
    {
    }

    bool GatelibraryProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
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

    bool GatelibraryProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
    {
        if (source_left.column() == 0 || source_left.column() == 2)
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
        else if (source_left.column() == 1)
            return source_left.data().toInt() < source_right.data().toInt();
        else if (source_left.column() == 3)
            return source_left.data(Qt::UserRole).toLongLong() < source_right.data(Qt::UserRole).toLongLong();
        else
        {
            return true;
        }

    }

    gui_utility::mSortMechanism GatelibraryProxyModel::sortMechanism()
    {
        return mSortMechanism;
    }

    void GatelibraryProxyModel::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
        invalidate();
    }

    void GatelibraryProxyModel::startSearch(QString text, int options)
    {
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }
}
