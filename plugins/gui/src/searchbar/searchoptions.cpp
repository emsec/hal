#include "gui/searchbar/searchoptions.h"
#include <QDebug>

namespace hal
{
    SearchOptions::SearchOptions(bool exactMatchCmb, bool caseSensitiveCmb, bool regExCmb, QList<int> cols)
        : mExactMatch(exactMatchCmb), mCaseSensitive(caseSensitiveCmb), mRegularExpression(regExCmb), mColumns(cols)
    {
        qInfo() << "constructor executed";
    }

    void SearchOptions::setOptions(bool exactMatch, bool caseSensitive, bool regEx, QList<int> columns)
    {
        mExactMatch = exactMatch;
        mCaseSensitive = caseSensitive;
        mRegularExpression = regEx;
        mColumns = columns;
    }
}
