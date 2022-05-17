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

#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "boolean_function_table_model.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"

#include <QLabel>
#include <QWidget>
#include <QVector>
#include <QTableWidget>


namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     *
     * @brief A table view that shows BooleanFunction%s and clear-preset behaviors.
     */
    class BooleanFunctionTable : public QTableView
    {
    Q_OBJECT

    public:
        /**
        * Constructor.
        *
        * @param parent - The parent widget
        */
        BooleanFunctionTable(QWidget* parent = nullptr);

        /**
         * Access the boolean function table model of this table.
         *
         * @returns the BooleanFunctionTableModel
         */
        BooleanFunctionTableModel* getModel();


    public Q_SLOTS:
        /**
         * Sets (and overwrites) the list of table entries that are displayed in this boolean function table.
         *
         * @param entries - The list of boolean function table entries
         */
        void setEntries(QVector<QSharedPointer<BooleanFunctionTableEntry>> entries);

    private Q_SLOTS:
        /**
         * Handles the resize event. The table is sized, so that the output column is bigger than the input columns.
         *
         * @param event - The QResizeEvent
         */
        void resizeEvent(QResizeEvent* event) override;

        void handleContextMenuRequest(const QPoint &pos);

    private:
        void adjustTableSizes();

        BooleanFunctionTableModel* mBooleanFunctionTableModel;
        Gate* mCurrentGate;
        u32 mCurrentGateId;

    };
} // namespace hal
