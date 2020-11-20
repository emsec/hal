#include "gui/grouping/grouping_proxy_model.h"

#include "gui/gui_globals.h"

namespace hal
{
    GroupingProxyModel::GroupingProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
    {
        mSortMechanism = gui_utility::mSortMechanism(gSettingsManager->get("navigation/mSortMechanism").toInt());
        connect(gSettingsRelay, &SettingsRelay::settingChanged, this, &GroupingProxyModel::handleGlobalSettingChanged);
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

            return gui_utility::compare(mSortMechanism, name_left, name_right);
        }
        else
            return source_left.data().toInt() < source_right.data().toInt();

    }

    void GroupingProxyModel::handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender);
        if (key == "navigation/mSortMechanism")
        {
            mSortMechanism = gui_utility::mSortMechanism(value.toInt());
            // force re-sort
            invalidate();
        }
    }
}
