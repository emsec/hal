//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include <QString>
#include <QList>
#include <QtCore/qobjectdefs.h>

namespace hal
{
    /**
     * @ingroup gui
     */
    namespace gui_utility
    {
        Q_NAMESPACE
		
        /**
         * The NumeratedString class extends the information of a normal QString by extracting
         * the in the QString contained numbers and saving these to a list. The remaining characters
         * are saved as an additional string.
         * Example: "a123bc9d" -> mNumList={123,9}, mRemainder="abcd"
         */
        class NumeratedString : public QString
        {
        public:
            QString mRemainder;
            QList<int> mNumList;
            NumeratedString(const QString& s);
        };

        /**
         * An enum for convenience purposes. Used in the compare function to determine which compare
         * criteria should be used.
         */
        enum mSortMechanism
        {
            lexical = 0,
            natural = 1,
            numerated = 2
        };

        Q_ENUM_NS(mSortMechanism);
		
        /**
         * Compares two strings by the natural order critera (e.g. z2 < z11).
         *
         * @param a - First string to compare.
         * @param b - Second string to compare.
         * @return True if a < b, false otherwise.
         */
        bool naturalOrderCompare(const QString& a, const QString& b);

        /**
         * Compares two strings by the lexical order critera (e.g. z11 < z2).
         *
         * @param a - First string to compare
         * @param b - Second string to compare
         * @return True if a < b, false otherwise.
         */
        bool lexicalOrderCompare(const QString& a, const QString& b);

        /**
         * Compares two strings firstly by their value without any digits.
         * If these parts are the same, the numbers the strings contained are compared.
         *
         * @param a - The first string to compare.
         * @param b - The second string to compare.
         * @return True if a < b, false otherwise.
         */
        bool numeratedOrderCompare(const QString& a, const QString& b);

        /**
         * Compares two string by its (numerical) value. The smaller string is filled
         * with "0"s until it matches the size of the longer string.
         *
         * @param a_num - First string to compare.
         * @param b_num - second string to compare.
         * @return An integer less than, equal to, or greater than zero if a_num is less than, equal to, or greater than b_num.
         */
        int numericStringCompare(QString a_num, QString b_num);

        /**
         * Compares two strings using a specific comparison criteria.
         *
         * @param mechanism - The criteria by which the 2 strings should be compared with.
         * @param a - The first string to compare.
         * @param b - The second string to compare.
         * @return True if a < b, false otherwise.
         */
        int compare(mSortMechanism mechanism, QString a, QString b);

    } // namespace gui_utility
}
