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
#include "groupings_of_item_model.h"

#include <QWidget>
#include <QTableWidget>


namespace hal
{

    class GroupingTableEntry;
    /**
     * @ingroup utility_widgets-selection_details
     * @brief A TableView to display all groupings that contain a specified Gate, Net or Module.
     *
     * This TableView uses a GroupingsOfItemModel as its model.
     * 
     * Using one of the methods setGate, setNet or setModule a Gate/Net/Module can be specified which Grouping%s
     * are displayed in the view afterwards.
     * 
     * The Model automatically keeps track of changes of the stored Grouping%s. It also adds/removes Grouping%s to/from the
     * model if the Grouping assigns/unassigns the observed item.  
     */
    class GroupingsOfItemWidget : public QTableView
    {
    Q_OBJECT

    public:

        /**
        * Constructor.
        *
        * @param parent - The parent widget
        */
        GroupingsOfItemWidget(QWidget* parent = nullptr);

        /**
         * Accesses the underlying GroupingsOfItemModel. 
         * 
         * @returns the underlying GroupingsOfItemModel.
         */
        GroupingsOfItemModel* getModel();

        /**
         * Specifies a Gate which Grouping%s are the content of this widget. 
         * 
         * @param gate - The specified Gate
         */
        void setGate(Gate* gate);

        /**
         * Specifies a Net which Grouping%s are the content of this widget. 
         * 
         * @param net - The specified Net
         */
        void setNet(Net* net);

        /**
         * Specifies a Module which Grouping%s are the content of this widget. 
         * 
         * @param module - The specified Module
         */
        void setModule(Module* module);

    Q_SIGNALS:
        void updateText(const QString& text);

    private Q_SLOTS:
        /**
         * Handles the resize event. The table is sized, so that the output column is bigger than the input columns.
         *
         * @param event - The QResizeEvent
         */
        void resizeEvent(QResizeEvent* event) override;

        void handleContextMenuRequest(const QPoint &pos);

        void handleLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);


    private:
        void updateAppearance();

        void adjustTableSizes();

        void notifyNewTitle(int elementCount);

        void changeNameTriggered(GroupingTableEntry entry);

        void changeColorTriggered(GroupingTableEntry entry);

        ItemType mCurrentObjectType;
        u32 mCurrentObjectId;
        GroupingsOfItemModel* mGroupingsOfItemModel;

        const QString mFrameTitleNoItem = "Groupings (0)";
        const QString mFrameTitleSingleItem = "Grouping";
        const QString mFrameTitleMultipleItems = "Groupings (%1)";

    };
} // namespace hal
