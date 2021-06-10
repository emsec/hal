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

#include "gui/new_selection_details_widget/models/lut_table_model.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"

#include <QTableView>


namespace hal
{

    class LUTTableWidget : public QTableView
    {
    Q_OBJECT

    public:
        /**
        * Constructor.
        *
        * @param parent - The parent widget
        */
        LUTTableWidget(QWidget* parent = nullptr);

        /**
         * Accesses the underlying table model of this LUTTableModel.
         *
         * @returns the LUTTableModel of this LUTTableWidget
         */
        LUTTableModel* getModel() const { return mLutModel; }

    public Q_SLOTS:
        /**
         * Handles that the focus in the selection details tree has been changed. Updates the currently displayed
         * LUT content if necessary.
         *
         * @param sti - The focused SelectionTreeItem
         */
        void handleDetailsFocusChanged(const SelectionTreeItem* sti);

    protected Q_SLOTS:
        /**
         * Handles the resize event. The table is sized, so that the output column is bigger than the input columns.
         *
         * @param event - The QResizeEvent
         */
        void resizeEvent(QResizeEvent* event) override;


    private:
        void updateGate(Gate* gate);
        void adjustColumnSizes();

        LUTTableModel* mLutModel;
        QTableView* mLutTableView;

    };
}
