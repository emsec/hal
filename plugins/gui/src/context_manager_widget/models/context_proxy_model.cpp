#include "gui/context_manager_widget/models/context_proxy_model.h"

#include "gui/gui_utils/sort.h"

#include <QDateTime>

namespace hal
{
    ContextProxyModel::ContextProxyModel(QObject* parent) : SearchProxyModel(parent)
    {

    }

    bool ContextProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        return checkRow(source_row, source_parent, 0, 1);
    }

    bool ContextProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        QVariant leftData = sourceModel()->data(left, Qt::UserRole);
        QVariant rightData = sourceModel()->data(right, Qt::UserRole);

        if(leftData.userType() == QMetaType::QDateTime)
            return leftData.toDateTime() > rightData.toDateTime();
        else
            return !(gui_utility::compare(gui_utility::mSortMechanism::natural, leftData.toString(), rightData.toString()));
    }

    void ContextProxyModel::startSearch(QString text, int options)
    {
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }
}
