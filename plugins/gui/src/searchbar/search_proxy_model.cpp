#include "gui/searchbar/search_proxy_model.h"

namespace hal {
    SearchProxyModel::SearchProxyModel()
        : opts(false, false, false, {})
    {

    }

    void SearchProxyModel::updateProxy(int options, QString text)
    {
        //opts.setOptions(exactMatch, caseSensitive, regEx, columns);
    }
}
