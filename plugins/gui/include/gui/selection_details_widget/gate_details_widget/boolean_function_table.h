// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

        /**
         * If set to true, the context menu entry shows "Boolean function to clipboard".
         * Otherwise it displays the actual name of the boolean function.
         *
         * @param enabled - False to show the actual function name, true otherwise.
         */
        void setContextMenuPlainDescr(bool enable){mShowPlainDescr = enable;}

        /**
         * If set to true, the context menu entry for the python code for shows "Get boolean function".
         * Otherwise it displays the actual name of the boolean function.
         *
         * @param enabled - False to show the actual function name, true otherse.
         */
        void setContextMenuPythonPlainDescr(bool enable){mShowPlainPyDescr = enable;}

        /**
         * If set to true, the context menu additionaly shows the "Change Boolean function" option.
         *
         * @param enable - True to show the entry, false otherwise.
         */
        void enableChangeBooleanFunctionOption(bool enable) {mChangeBooleanFunc = enable;}


    public Q_SLOTS:
        /**
         * Sets (and overwrites) the list of table entries that are displayed in this boolean function table.
         *
         * @param entries - The list of boolean function table entries
         */
        void setEntries(QVector<QSharedPointer<BooleanFunctionTableEntry>> entries);

        /**
         * Sets the id (and the gate) as internal information so the context menu has access to its full functionality.
         * To "reset" this information in case no gate is explicity displayed, a nullptr can be given as a parameter.
         * This function is "optional" as long as the context menu entry "Change Boolean function" is not needed since
         * the table is filled through the setEntries method.
         *
         * @param g - The gate to set.
         */
        void setGateInformation(Gate* g);

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

        bool mShowPlainDescr;
        bool mShowPlainPyDescr;
        bool mChangeBooleanFunc;

    };
} // namespace hal
