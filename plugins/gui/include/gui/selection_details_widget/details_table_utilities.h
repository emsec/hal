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
#include <QSize>

class QTableView;
class QTableWidget;

namespace hal {

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Helper class consisting of usefull details-related functions.
     *
     * A utility class used to gather details-tables related helperfunctions.
     */
    class DetailsTableUtilities
    {
    public:
        //utility function, used to calculate the actual width so the scrollbars and the accuracy of the click functionality is correct
        /**
         * Calculates the "actual" width and height for the given table.
         *
         * @param table - The view for which to calculate the size.
         * @param nrows - The view's (model's) number of rows.
         * @param ncols - The view's (model's) number of columns.
         * @return The calculated size of the table.
         */
        static QSize tableViewSize(const QTableView* table, int nrows, int ncols);

        /**
         * Caluclates the "actual" width and height for the given table.
         *
         * @param table - The table for which to calculate the size.
         * @return The calculated size of the table.
         */
        static QSize tableWidgetSize(const QTableWidget* table);

        /**
         * Sets some basic properties of the table that are the same for all of the them.
         *
         * @param table - The table for which to set the default properties.
         */
        static void setDefaultTableStyle(QTableView* table);
    };
}
