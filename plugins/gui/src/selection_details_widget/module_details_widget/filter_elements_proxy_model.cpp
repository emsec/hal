#include "gui/selection_details_widget/module_details_widget/filter_elements_proxy_model.h"
#include "gui/module_model/module_item.h"

#include "gui/gui_globals.h"

namespace hal
{
    FilterElementsProxyModel::FilterElementsProxyModel(QObject* parent)
    {}

    void FilterElementsProxyModel::setFilterNets(bool filterNets){
        mFilterNets = filterNets;
        invalidateFilter();
    }
    
    void FilterElementsProxyModel::setFilterGates(bool filterGates){
        mFilterGates = filterGates;
        invalidateFilter();
    }

    bool FilterElementsProxyModel::areNetsFiltered(){
        return mFilterNets;
    }

    bool FilterElementsProxyModel::areGatesFiltered(){
        return mFilterGates;
    }

    bool FilterElementsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
        auto item = static_cast<ModuleItem*>(sourceIndex.internalPointer());

        if(mFilterNets && item->getType() == ModuleItem::TreeItemType::Net)
            return false;
        if(mFilterGates && item->getType() == ModuleItem::TreeItemType::Gate)
            return false;

        return true;
    }
}
