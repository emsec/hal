#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    SelectionTreeProxyModel::SelectionTreeProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
    {
        m_sort_mechanism = gui_utility::sort_mechanism(g_settings_manager->get("navigation/sort_mechanism").toInt());
        m_filter_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &SelectionTreeProxyModel::handle_global_setting_changed);
    }

    bool SelectionTreeProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        //used as parent index if item has children
        const QModelIndex& item_name_index = sourceModel()->index(source_row, 0, source_parent);
 
        //check all children of the current item and keep it if one child satisfies the match
        for(int i = 0; i < sourceModel()->rowCount(item_name_index); i++)
        {
            if(filterAcceptsRow(i, item_name_index))
                return true;
        }
        
        const QModelIndex& item_id_index = sourceModel()->index(source_row, 1, source_parent);
        const QModelIndex& item_type_index = sourceModel()->index(source_row, 2, source_parent);

        const QString& item_name = item_name_index.data().toString();
        const QString& item_id = item_id_index.data().toString();
        const QString& item_type = item_type_index.data().toString();

        if(m_filter_expression.match(item_type).hasMatch() || m_filter_expression.match(item_name).hasMatch() || m_filter_expression.match(item_id).hasMatch())
            return true;
        else
            return false;
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

    void SelectionTreeProxyModel::handle_filter_text_changed(const QString& filter_text)
    {
        m_filter_expression.setPattern(filter_text);
        invalidateFilter();
    }
}
