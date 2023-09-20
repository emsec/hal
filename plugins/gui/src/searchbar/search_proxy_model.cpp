#include "gui/searchbar/search_proxy_model.h"

namespace hal {
    SearchProxyModel::SearchProxyModel()
        : opts(false, false, false, {})
    {

    }

    void SearchProxyModel::updateProxy(bool exactMatch, bool caseSensitive, bool regEx, QList<int> columns)
    {
        opts.setOptions(exactMatch, caseSensitive, regEx, columns);
    }
}
