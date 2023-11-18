#include "gui/searchbar/search_proxy_model.h"
#include <QDebug>

namespace hal {
    SearchProxyModel::SearchProxyModel(QObject* parent): QSortFilterProxyModel(parent)
    {
        mSearchOptions = SearchOptions(8); // ? search only first column ?
    }

    void SearchProxyModel::startSearch(QString text, int options)
    {
        mSearchString  = text;
        mSearchOptions = SearchOptions(options);
    }

    bool SearchProxyModel::isMatching(const QString searchString, const QString stringToCheck) const
    {
        if(!mSearchOptions.isExactMatch() && !mSearchOptions.isRegularExpression()){
            //check if stringToCheck contains the searchString
            return stringToCheck.contains(searchString, mSearchOptions.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }
        else if(mSearchOptions.isExactMatch()){
            //check if the stringToCheck is the same as the searchString   - also checks CaseSensitivity

            return 0 == QString::compare(searchString, stringToCheck, mSearchOptions.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }
        else if(mSearchOptions.isRegularExpression()){
            //checks if the stringToCheck matches the regEx given by searchString
            auto regEx = QRegularExpression(searchString, mSearchOptions.isCaseSensitive() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
            return regEx.match(stringToCheck).hasMatch();
        }
        return false;
    }

    QList<QString> SearchProxyModel::getColumnNames()
    {
        QList<QString> list = QList<QString>();
        for(int column = 0; column < this->columnCount(); column++){
            QVariant header = this->headerData(column, Qt::Horizontal, Qt::DisplayRole);
            if(header.isValid() && header.canConvert<QString>()){
                list.append(header.toString());
            }
        }
        return list;
    }

    bool SearchProxyModel::checkRowRecursion(int sourceRow, const QModelIndex &sourceParent,
                                             int startIndex, int endIndex, int offset) const
    {
        if (checkRow(sourceRow, sourceParent, startIndex, endIndex, offset))
            return true;
        QModelIndex currentIndex = sourceModel()->index(sourceRow, 0, sourceParent);
        int nrows = sourceModel()->rowCount(currentIndex);
        for (int irow = 0; irow < nrows; irow++)
        {
            if (checkRowRecursion(irow, currentIndex, startIndex, endIndex, offset))
                return true;
        }
        return false;
    }

    bool SearchProxyModel::checkRow(int sourceRow, const QModelIndex& sourceParent, int startIndex, int endIndex, int offset) const
    {
        QList<int> columns = mSearchOptions.getColumns();
        if(columns.empty()){
            //iterate over each column
            for(int index = startIndex; index <= endIndex; index++){
                QString entry = sourceModel()->index(sourceRow, index, sourceParent).data().toString();
                if(isMatching(mSearchString, entry))
                {
                    return true;
                }
            }
            return false;
        }
        else
        {
            for(int index : columns)
            {
                QString entry = sourceModel()->index(sourceRow, index + offset, sourceParent).data().toString();
                if(isMatching(mSearchString, entry))
                    return true;
            }
            return false;
        }

    }

}
