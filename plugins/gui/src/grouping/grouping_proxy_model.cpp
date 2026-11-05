



#include "gui/grouping/grouping_proxy_model.h"

#include <QColor>

#include "gui/gui_globals.h"

namespace hal
{
    GroupingProxyModel::GroupingProxyModel(QObject* parent) : SearchProxyModel(parent), mSortMechanism(gui_utility::mSortMechanism::natural)
    {

    }

    bool GroupingProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        return checkRow(sourceRow, sourceParent, 0, 2);
    }

    bool GroupingProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
    {
        // column 0: name, column 1: ID, column 2: color
        switch (source_left.column())
        {
            case 1:
                return source_left.data().toInt() < source_right.data().toInt();
            case 2: {
                // sort by hue, then saturation, then value (plain hex codes would sort by red component only)
                const QColor color_left  = source_left.data(Qt::BackgroundRole).value<QColor>();
                const QColor color_right = source_right.data(Qt::BackgroundRole).value<QColor>();
                if (color_left.hsvHue() != color_right.hsvHue())
                    return color_left.hsvHue() < color_right.hsvHue();
                if (color_left.hsvSaturation() != color_right.hsvSaturation())
                    return color_left.hsvSaturation() < color_right.hsvSaturation();
                return color_left.value() < color_right.value();
            }
            default: {
                QString name_left  = source_left.data().toString();
                QString name_right = source_right.data().toString();

                if (sortCaseSensitivity() == Qt::CaseInsensitive)
                {
                    name_left  = name_left.toLower();
                    name_right = name_right.toLower();
                }

                return gui_utility::compare(mSortMechanism, name_left, name_right);
            }
        }

    }

    gui_utility::mSortMechanism GroupingProxyModel::sortMechanism()
    {
        return mSortMechanism;
    }

    void GroupingProxyModel::setSortMechanism(gui_utility::mSortMechanism sortMechanism)
    {
        mSortMechanism = sortMechanism;
        invalidate();
    }
    void GroupingProxyModel::startSearch(QString text, int options)
    {
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }
}
