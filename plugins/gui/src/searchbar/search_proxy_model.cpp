#include "gui/searchbar/search_proxy_model.h"
#include <QDebug>

namespace hal {
    SearchProxyModel::SearchProxyModel(QObject* parent): QSortFilterProxyModel(parent)
    {
        mSearchOptions = new SearchOptions();
    }

    void SearchProxyModel::startSearch(QString text, int options)
    {
        mSearchOptions = new SearchOptions(options);
        qInfo() << "Proxy received searchOptions";
    }

}
