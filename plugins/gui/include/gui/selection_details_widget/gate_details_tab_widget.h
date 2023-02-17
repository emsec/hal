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

#include "gui/selection_details_widget/details_tab_widget.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "gui/selection_details_widget/gate_details_widget/boolean_function_table.h"
#include "gui/selection_details_widget/gate_details_widget/lut_table_widget.h"
#include "gui/selection_details_widget/data_table_widget.h"
#include "gui/selection_details_widget/groupings_of_item_widget.h"

namespace hal
{
    class Gate;
    class DetailsFrameWidget;
    class GateInfoTable;
    class GatePinTree;
    class BooleanFunctionTable;
    class LUTTableWidget;
    class DataTableWidget;
    class GroupingsOfItemWidget;
    class CommentWidget;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief The DetailsTabWidget that is responsible for showing Gate details
     * 
     * This tab widget contains and manages the tabs that are shown when selecting a Gate in the SelectionDetailsWidet's tree.
     */
    class GateDetailsTabWidget : public DetailsTabWidget
    {
        Q_OBJECT

    private:
        enum GateTypeCategory {none, lut, ff, latch};

    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
         GateDetailsTabWidget(QWidget* parent = nullptr);

        /**
         * Sets the gate which details are shown in the tabs and various widgets.
         *
         * @param gate - The gate.
         */
        void setGate(Gate* gate);

    private Q_SLOTS:

        void handleGateBooleanFunctionChanged(Gate* g);


    private:
        /**
         * Shows the tab "(LUT / FF / LATCH)" and their corresponding type widgets depending on which gate type property is provided.
         * 
         * @param gateTypeProperty - The category type the gate is of (i.e. LUT, latch, FF or Nothing).
         */
        void showMultiTab(GateDetailsTabWidget::GateTypeCategory gateTypeCategory);

        /**
         * Hides the "(LUT / FF / LATCH)" tab.
         */
        void hideMultiTab();

        /**
         * Checks wheter the "(LUT / FF / LATCH)" tab must be hidden or shown depending on gate and changes the visibility of the tab depending on the result of the check.
         *
         * @param gateTypeCategory - The gate category the gate that should be shown is of. Decides which tab is shown.
         */
        void hideOrShorMultiTab(GateTypeCategory gateTypeCategory);

        /**
         * Collects the gate's boolean functions and setups the boolean function tables and the lut truth table.
         */
        void setupBooleanFunctionTables(Gate* gate, GateTypeCategory gateTypeCategory);

        /**
         * Gets the category the gate belongs to (i.e. LUT / FF / LATCH / Nothing).
         */
        GateTypeCategory getGateTypeCategory(Gate* gate) const;

        /**
         * Shows the requested context menu. It is connected to the LutConfigLabel.
         */
        void handleLutConfigContextMenuRequested(QPoint pos);//might make an own class for the configstring, context menu is handled there?

        bool mMultiTabVisible = true; 
        int mMultiTabIndex;
        QWidget* mMultiTabContent;

        Gate* mCurrentGate;

        //general tab
        GateInfoTable* mGateInfoTable;
        DetailsFrameWidget* mGateInformationFrame;

        //groupings tab
        GroupingsOfItemWidget* mGroupingsOfItemTable;
        DetailsFrameWidget* mGroupingsFrame;

        //pins tab
        GatePinTree* mPinsTree;
        DetailsFrameWidget* mPinsFrame;

        //ff tab
        BooleanFunctionTable* mFfFunctionTable;
        DetailsFrameWidget* mFfFrame;

        //latch tab
        BooleanFunctionTable* mLatchFunctionTable;
        DetailsFrameWidget* mLatchFrame;

        //lut tab
        BooleanFunctionTable* mLutFunctionTable;
        DetailsFrameWidget* mLutFrame;

        QLabel* mLutConfigLabel;
        DetailsFrameWidget* mLutConfigurationFrame;

        LUTTableWidget* mLutTable;
        DetailsFrameWidget* mTruthTableFrame;

        //boolean function tab
        BooleanFunctionTable* mFullFunctionTable;
        DetailsFrameWidget* mBooleanFunctionsFrame;

        //data tab
        DataTableWidget* mDataTable;
        DetailsFrameWidget* mDataFrame;

        //comment tab
        CommentWidget* mCommentWidget;
    };
}
