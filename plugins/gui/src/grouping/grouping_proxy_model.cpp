#include "gui/grouping/grouping_proxy_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    GroupingProxyModel::GroupingProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
    {
        m_sort_mechanism = gui_utility::sort_mechanism(g_settings_manager->get("navigation/sort_mechanism").toInt());
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &GroupingProxyModel::handle_global_setting_changed);
    }

    bool GroupingProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        if(!filterRegExp().isEmpty())
        {
            QModelIndex source_index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
            //QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);
            if(source_index.isValid())
            {
                if (sourceModel()->data(source_index, filterRole()).toString().contains(filterRegExp()))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool GroupingProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
    {
        // FILTERKEYCOLUMN SHOULD BE EITHER 0 OR 1

        if (filterKeyColumn() == 0)
        {
            QString name_left = source_left.data().toString();
            QString name_right = source_right.data().toString();

            if (sortCaseSensitivity() == Qt::CaseInsensitive)
            {
                name_left = name_left.toLower();
                name_right = name_right.toLower();
            }

            return gui_utility::compare(m_sort_mechanism, name_left, name_right);
        }
        else
            return source_left.data().toInt() < source_right.data().toInt();

    }

    void GroupingProxyModel::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
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
