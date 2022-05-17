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
#include "data_table_model.h"

#include "hal_core/netlist/gate.h"

#include <QWidget>
#include <QTableWidget>


namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     *
     * @brief A widget to display the data of a DataContainer (Module, Gate or Net)
     * 
     * A widget to display the data of a DataContainer (Module, Gate or Net) using the DataTableModel. 
     * The DataContainer to be shown can be configured via setGate/setModule/setNet. Is is currently not
     * possible to observe data changes since no corresponding signal is implemented in the HAL core.
     */
    class DataTableWidget : public QTableView
    {
    Q_OBJECT

    public:

        /**
        * Constructor.
        *
        * @param parent - The parent widget
        */
        DataTableWidget(QWidget* parent = nullptr);

        /**
         * Accesses the undelying data table model used by the QTableView. 
         * 
         * @returns the underlying data table model.
         */
        DataTableModel* getModel();

    public Q_SLOTS:

        /**
         * Configures the DataTableWidget to show the data of the specified gate.
         * 
         * @param gate - The specified gate 
         */
        void setGate(Gate* gate);

        /**
         * Configures the DataTableWidget to show the data of the specified net.
         * 
         * @param net - The specified net 
         */
        void setNet(Net* net);

        /**
         * Configures the DataTableWidget to show the data of the specified module.
         * 
         * @param module - The specified module 
         */
        void setModule(Module* module);

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

        enum DataContainerType
        {
            GATE,
            NET,
            MODULE,
            DATA_CONTAINER // <- A class that is simply a data container
        };

        DataTableModel* mDataTableModel;
        DataContainerType mCurrentObjectType;
        u32 mCurrentObjectId;
    };
} // namespace hal
