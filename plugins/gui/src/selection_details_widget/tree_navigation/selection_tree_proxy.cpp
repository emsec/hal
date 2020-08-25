#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"

#include "gui/gui_globals.h"

namespace hal
{
    SelectionTreeProxyModel::SelectionTreeProxyModel(QObject* parent)
        : QSortFilterProxyModel(parent), mGraphicsBusy(0)
    {
        m_sort_mechanism = gui_utility::sort_mechanism(g_settings_manager->get("navigation/sort_mechanism").toInt());
        m_filter_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &SelectionTreeProxyModel::handle_global_setting_changed);
    }

    bool SelectionTreeProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        //index to element in source mdoel
        const QModelIndex& itemIndex = sourceModel()->index(source_row, 0, source_parent);
 
        const SelectionTreeItem* sti = static_cast<SelectionTreeItem*>(itemIndex.internalPointer());
        return sti->match(m_filter_expression);
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

        bool comparison = gui_utility::compare(m_sort_mechanism, name_left, name_right);

        // if left element is a structure element (these must be handled specially)
        // (right element must then also be a structure element)
        if (source_left.data(Qt::UserRole).toBool())
        {
            // forces "Gates" to be before "Nets" regardless of sort order
            comparison ^= (sortOrder() == Qt::AscendingOrder);
        }

        return comparison;
    }

    void SelectionTreeProxyModel::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender);
        if (key == "navigation/sort_mechanism")
        {
            m_sort_mechanism = gui_utility::sort_mechanism(value.toInt());
            // force re-sort
            invalidate();
        }
    }

    void SelectionTreeProxyModel::applyFilterOnGraphics()
    {
        if (isGraphicsBusy()) return;
        ++ mGraphicsBusy;
        QList<u32> modIds;
        QList<u32> gatIds;
        QList<u32> netIds;
        static_cast<const SelectionTreeModel*>(sourceModel())->suppressedByFilter(modIds, gatIds, netIds, m_filter_expression);
        g_selection_relay->suppressedByFilter(modIds, gatIds, netIds);
        -- mGraphicsBusy;
    }

    void SelectionTreeProxyModel::handle_filter_text_changed(const QString& filter_text)
    {
        m_filter_expression.setPattern(filter_text);
        invalidateFilter();
        applyFilterOnGraphics();
    }
}
