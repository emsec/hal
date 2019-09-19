#include "selection_details_widget/tree_navigation/tree_module_proxy_model.h"

tree_module_proxy_model::tree_module_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
{
}

bool tree_module_proxy_model::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (!filterRegExp().isEmpty())
    {
        QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
        if (source_index.isValid())
        {
            int child_count = sourceModel()->rowCount(source_index);
            for (int i = 0; i < child_count; i++)
            {
                if (filterAcceptsRow(i, source_index))
                    return true;
            }
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
