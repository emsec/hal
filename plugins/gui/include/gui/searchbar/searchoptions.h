#ifndef SEARCHOPTIONS_H
#define SEARCHOPTIONS_H

#include "qsortfilterproxymodel.h"
#include <QSortFilterProxyModel>

#endif // SEARCHOPTIONS_H

namespace hal
{
    class SearchOptions : public QSortFilterProxyModel
    {
        Q_OBJECT
    public:
        SearchOptions();
    Q_SIGNALS:
        void emitStartSearch();
    public Q_SLOTS:
        void startSearch(const QString& text, int options);
    private:
        bool mExactMatch;
        bool mCaseSensitive;
        bool mRegularExpression;
        QList<int> mColumns;
    };
}
