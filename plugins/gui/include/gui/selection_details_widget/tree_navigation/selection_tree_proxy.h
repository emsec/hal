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
#include <QRegularExpression>

namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     * @brief Enables filtering of the SelectionTreeModel.
     *
     * A proxy model to filter the SelectionTreeModel. This allows to search efficiently through the
     * model and the results can be displayed within the view in a tree-styled fashion.
     */
    class SelectionTreeProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT
    public:

        /**
         * The constructor.
         *
         * @param parent - The model's parent.
         */
        SelectionTreeProxyModel(QObject* parent = nullptr);

        /**
         * Calls the suppressedByFilter() function to get all items that should not be displayed
         * in the view because they do not match the filter-string. Then it tells the selection
         * relay to update these items.
         */
        void applyFilterOnGraphics();

        /**
         * Checks if the model is still busy with applying the changes (in applyFilterOnGraphics()).
         *
         * @return True if the model is still busy. False otherwise.
         */
        bool isGraphicsBusy() const { return mGraphicsBusy > 0; }

        /**
         * Get the currently set sort mechanism.
         *
         * @return The sorting mechanism.
         */
        gui_utility::mSortMechanism sortMechanism();

        /**
         * Sets the given sort mechanism.
         *
         * @param sortMechanism - The new mechanism.
         */
        void setSortMechanism(gui_utility::mSortMechanism sortMechanism);

    protected:

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    public Q_SLOTS:

        /**
         * Filters the model by the given text.
         *
         * @param filter_text - The text to filter the model by.
         */
        void handleFilterTextChanged(const QString& filter_text);

    private:
        gui_utility::mSortMechanism mSortMechanism;
        QRegularExpression mFilterExpression;
        int mGraphicsBusy;
    };
}
