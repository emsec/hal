#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"

#include "gui/gui_globals.h"

namespace hal
{
    SelectionTreeProxyModel::SelectionTreeProxyModel(QObject* parent)
        : QSortFilterProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::lexical), mGraphicsBusy(0)
    {
        mFilterExpression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    bool SelectionTreeProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        //index to element in source mdoel
        const QModelIndex& itemIndex = sourceModel()->index(source_row, 0, source_parent);
 
        const SelectionTreeItem* sti = static_cast<SelectionTreeItem*>(itemIndex.internalPointer());
        return sti->match(mFilterExpression);
    }

    bool SelectionTreeProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
    {
        QString name_left = source_left.data().toString();
        QString name_right = source_right.data().toString();
        if (sortCaseSensitivity() == Qt::CaseInsensitive)
        {
            name_left = name_left.toLower();
            name_right = name_right.toLower();
        }

        bool comparison = gui_utility::compare(mSortMechanism, name_left, name_right);

        // if left element is a structure element (these must be handled specially)
        // (right element must then also be a structure element)
        if (source_left.data(Qt::UserRole).toBool())
        {
            // forces "Gates" to be before "Nets" regardless of sort order
            comparison ^= (sortOrder() == Qt::AscendingOrder);
        }

        return comparison;
    }

    void SelectionTreeProxyModel::applyFilterOnGraphics()
    {
        if (isGraphicsBusy()) return;
        ++ mGraphicsBusy;
        QList<u32> modIds;
        QList<u32> gatIds;
        QList<u32> netIds;
        static_cast<const SelectionTreeModel*>(sourceModel())->suppressedByFilter(modIds, gatIds, netIds, mFilterExpression);
        gSelectionRelay->suppressedByFilter(modIds, gatIds, netIds);
        -- mGraphicsBusy;
    }

    void SelectionTreeProxyModel::handleFilterTextChanged(const QString& filter_text)
    {
        mFilterExpression.setPattern(filter_text);
        invalidateFilter();
        applyFilterOnGraphics();
    }

    gui_utility::mSortMechanism SelectionTreeProxyModel::sortMechanism()
    {
        return mSortMechanism;
    }

    void SelectionTreeProxyModel::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
        invalidate();
    }
}
