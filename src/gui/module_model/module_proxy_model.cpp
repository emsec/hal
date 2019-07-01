#include "module_model/module_proxy_model.h"

module_proxy_model::module_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
{

}

bool module_proxy_model::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if(!filterRegExp().isEmpty())
    {
        QModelIndex source_index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
        if(source_index.isValid())
        {
            int children = sourceModel()->rowCount(source_index);
            for(int i = 0; i < children; ++i)
            {
                if(filterAcceptsRow(i, source_index))
                    return true;
            }

            return sourceModel()->data(source_index, filterRole()).toString().contains(filterRegExp());
        }
    }

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
