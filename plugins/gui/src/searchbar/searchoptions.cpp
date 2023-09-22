#include "gui/searchbar/searchoptions.h"
#include <QDebug>


namespace hal
{
    SearchOptions::SearchOptions()
    {};

    SearchOptions::SearchOptions(int code)
    {
        mExactMatch = code & 1;
        mCaseSensitive = code & 2;
        mRegularExpression = code & 4;
        if (!(code & (1<<3))) //check if the first column is 0
        {
            for(int i = 4; i<32; i++)
            {
                if(code & (1<<i))
                    mColumns.append(i-4);
            }
        }

        qInfo() << mExactMatch << mCaseSensitive << mRegularExpression << "; " << mColumns;
    }

    int SearchOptions::toInt() const
    {
        int prefix = 0;
        int suffix = 0;
        prefix += mExactMatch;
        prefix += mCaseSensitive << 1;
        prefix += mRegularExpression << 2;
        if(mColumns.empty()) prefix += 1 << 3;
        else
        {
            for(int i=0; i < mColumns.length(); i++)
            {
                suffix += 1<<mColumns[i];
            }
        }
        suffix =  suffix << 4;
        return prefix + suffix;
    }

    int SearchOptions::toInt(bool exactMatch, bool caseSensitive, bool regEx, QList<int> columns)
    {
        int prefix = 0;
        int suffix = 0;
        prefix += exactMatch;
        prefix += caseSensitive << 1;
        prefix += regEx << 2;
        if(columns.empty()) prefix += 1 << 3;
        else
        {
            for(int i=0; i < columns.length(); i++)
            {
                suffix += 1<<columns[i];
            }
        }
        suffix =  suffix << 4;
        return prefix + suffix;
    }

    void SearchOptions::setOptions(bool exactMatch, bool caseSensitive, bool regEx, QList<int> columns)
    {
        mExactMatch = exactMatch;
        mCaseSensitive = caseSensitive;
        mRegularExpression = regEx;
        mColumns = columns;
    }
}
