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

#include "gui/gui_utils/sort.h"

#include <QSortFilterProxyModel>

namespace hal
{
    /**
     * @ingroup utility_widgets-grouping
     * @brief Helper model to make filtering available.
     *
     * The QSortFilterProxyModel set between the view of the GroupingManagerWidget and the
     * GroupingTableModel (source model) to support data filtering.
     * It is used to provide a search bar (<i>CTRL+F</i>) for groupings.
     */
    class GroupingProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent object
         */
        GroupingProxyModel(QObject* parent = nullptr);

        /**
         * Gets the mSortMechanism that should be applied to sort groupings (by their name).
         *
         * @returns the currently configured sorting mechanism
         */
        gui_utility::mSortMechanism sortMechanism();

        /**
         * Configures the mSortMechanism that should be applied to sort groupings (by their name).
         *
         * @param sortMechanism - the new sorting mechanism
         */
        void setSortMechanism(gui_utility::mSortMechanism sortMechanism);

    protected:
        /**
         * Overrides QSortFilterProxyModel::filterAcceptsRow to implement the filter logic based on the regular
         * expression stored by setFilterRegularExpression.<br>
         * Returns <b>true</b> if the item in the row indicated by <i>sourceRow</i> and <i>sourceParent</i> should be included
         * in the model.
         *
         * @param sourceRow - The row in the source model
         * @param sourceParent - the source parent
         * @returns <b>true</b> iff the row should be included in the model.
         */
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

        /**
         * Implements a comparison operator used for sorting. In this case it is based on the grouping names.
         *
         * @param source_left - The model index of the left element
         * @param source_right - The model index of the right element
         * @returns <b>true</b> iff the element at source_left is considered less than the element at source_right
         */
        bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    private:
        gui_utility::mSortMechanism mSortMechanism;
    };
}
