#ifndef SEARCHOPTIONS_H
#define SEARCHOPTIONS_H

#include "qsortfilterproxymodel.h"

#endif // SEARCHOPTIONS_H

namespace hal
{
    class SearchOptions : public QSortFilterProxyModel
    {
    public:

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted as additional information when the search gets started
         * @param text - searched text
         * @param options - search options, such as exact match, case sensitive, regular expression or columns
         */
        void startSearch(const QString& text, int options);
    private:
        bool mExactmatch;
        bool mCaseSensitive;
        bool mRegularExpression;
        QList <int> mColumns;
    };
}
