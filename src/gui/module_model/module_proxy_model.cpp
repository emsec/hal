#include "gui/module_model/module_proxy_model.h"
#include "gui/module_model/module_item.h"

#include "gui/gui_globals.h"

namespace hal
{
    module_proxy_model::module_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
    {
        // QTS PROXY MODELS ARE DUMB, IMPLEMENT CUSTOM SOLUTION OR SWITCH TO A DIFFERENT FILTER METHOD

        // IN VIEW
        // EVERY TIME FILTER CHANGES / ITEM GETS ADDED MODIFY LOCAL DATA STRUCTURE TO REFLECT CURRENT ITEM VISUALS
        // STYLED DELEGATES USE THAT DATA STRUCTURE TO DRAW THEMSELVES

        m_sort_mechanism = gui_utility::sort_mechanism(g_settings_manager.get("navigation/sort_mechanism").toInt());
        connect(&g_settings_relay, &settings_relay::setting_changed, this, &module_proxy_model::handle_global_setting_changed);
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

    bool module_proxy_model::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
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

    void module_proxy_model::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
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
