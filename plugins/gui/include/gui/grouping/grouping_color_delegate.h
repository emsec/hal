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

#include <QStyledItemDelegate>

namespace hal {
    /**
     * @ingroup utility_widgets-grouping
     * @brief Displays a colored box for the GroupingManagerWidget.
     *
     * QStyledItemDelegate used for displaying a colored box in the 'Color'-column
     * within the grouping_manager_widget table.
     */
    class GroupingColorDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        GroupingColorDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {;}

        /**
         * Paints the colored box in the table cell.
         *
         * @param painter - The used painter
         * @param option - The used style options
         * @param index - The index of the grouping within the GroupingTableModel
         */
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        /**
         * Returns the sized needed by the delegate to display the colored box.
         *
         * @param option - The used style options
         * @param index - The index of the grouping within the GroupingTableModel
         * @returns he sized needed by the delegate to display the colored box
         */
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    };
}
