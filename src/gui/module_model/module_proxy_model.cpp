#include "gui/module_model/module_proxy_model.h"

#include "gui/module_model/module_item.h"

module_proxy_model::module_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
{
    // QTS PROXY MODELS ARE DUMB, IMPLEMENT CUSTOM SOLUTION OR SWITCH TO A DIFFERENT FILTER METHOD

    // IN VIEW
    // EVERY TIME FILTER CHANGES / ITEM GETS ADDED MODIFY LOCAL DATA STRUCTURE TO REFLECT CURRENT ITEM VISUALS
    // STYLED DELEGATES USE THAT DATA STRUCTURE TO DRAW THEMSELVES
}

bool module_proxy_model::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if(!filterRegExp().isEmpty())
    {
        //QModelIndex source_index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
        QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);
        if(source_index.isValid())
        {
            if (sourceModel()->data(source_index, filterRole()).toString().contains(filterRegExp()))
            {
                static_cast<module_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(true);
                return true;
            }
            else
            {
                static_cast<module_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(false);
                return false;
            }
        }
    }

    static_cast<module_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(false);
    return true;
    //return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
