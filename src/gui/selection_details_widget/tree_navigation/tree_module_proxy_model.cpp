#include "selection_details_widget/tree_navigation/tree_module_proxy_model.h"
#include "selection_details_widget/tree_navigation/tree_module_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    TreeModuleProxyModel::TreeModuleProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
    {
        m_sort_mechanism = gui_utility::sort_mechanism(g_settings_manager.get("navigation/sort_mechanism").toInt());
        connect(&g_settings_relay, &SettingsRelay::setting_changed, this, &TreeModuleProxyModel::handle_global_setting_changed);
    }

    bool TreeModuleProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
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

    bool TreeModuleProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
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

    void TreeModuleProxyModel::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender);
        if (key == "navigation/sort_mechanism")
        {
            m_sort_mechanism = gui_utility::sort_mechanism(value.toInt());
            // force re-sort
            invalidate();
        }
    }
}
