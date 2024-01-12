#include "gui/pin_model/pin_proxy_model.h"

#include "gui/gui_utils/sort.h"

#include <QDateTime>

namespace hal
{
    PinProxyModel::PinProxyModel(QObject* parent) : SearchProxyModel(parent)
    {

    }

    bool PinProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        return checkRow(source_row, source_parent, 0, 1);
    }


    void PinProxyModel::startSearch(QString text, int options)
    {
        mSearchString = text;
        mSearchOptions = SearchOptions(options);
        invalidateFilter();
    }
}
