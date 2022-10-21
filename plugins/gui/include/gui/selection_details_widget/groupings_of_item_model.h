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

#include <QAbstractTableModel>
#include <QString>
#include <QColor>
#include <QList>

#include "hal_core/defines.h"
#include "hal_core/netlist/grouping.h"
#include "gui/gui_def.h"
#include "gui/validator/validator.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/grouping/grouping_table_model.h"

namespace hal {

    /**
     * @ingroup gui
     * @brief A TableModel to store all Grouping%s that contain a specified Gate, Net or Module.
     *
     * Using one of the methods setGate, setNet or setModule a Gate/Net/Module can be specified which Grouping%s
     * are stored in this model afterwards. 
     * 
     * Each row of the model contains the entries [Grouping Name, Grouping ID, Grouping Color].
     * 
     * The Model automatically keeps track of changes of the stored Grouping%s. It also adds/removes Grouping%s to/from the
     * model if the Grouping assigns/unassigns the observed item.  
     */
    class GroupingsOfItemModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        GroupingsOfItemModel(QObject* parent=nullptr);

        /**
         * Returns the amount of columns. Is always 3 (Grouping Name, Grouping ID, Grouping Color)
         *
         * @param parent - Then parent model index
         * @returns 3
         */
        int columnCount(const QModelIndex &parent=QModelIndex()) const override;

        /**
         * Returns the amount of rows in the table. Is equal to the amount of stored groupings.
         *
         * @param parent - The parent model index
         * @returns the amount of rows in the table.
         */
        int rowCount(const QModelIndex &parent=QModelIndex()) const override;

        /**
         * Returns the data stored under the given role for a given index in the table model.
         *
         * @param index - The index in the table
         * @param role - The access role
         * @returns the data
         */
        QVariant data(const QModelIndex &index, int role) const override;

        /**
         * Returns the header data fields, i.e. 'Groupings', 'ID' or 'Color'
         *
         * @param section - The section (column) index
         * @param orientation - The orientation of the table
         * @param role - The access role
         * @returns the header data field at the given position
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        /**
         * Sets the role data for the item at index to value.
         *
         * @param index - The model index
         * @param value - The value to set
         * @param role - The access role
         * @returns <b>true</b> on success
         */
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        /**
          * Gets the table entry that contains the grouping at a given row index.
          *
          * @param irow - The row index
          * @returns the grouping table entry
          */
        GroupingTableEntry getGroupingEntryAtRow(int row) const { return mGroupings.at(row); }

        /**
         * Specifies a Gate which Grouping%s are the content of this model. 
         * 
         * @param gate - The specified Gate
         */
        void setGate(Gate* gate);

        /**
         * Specifies a Module which Groupings are the content of this model. 
         * 
         * @param module - The specified Module
         */
        void setModule(Module* module);

        /**
         * Specifies a Net which Groupings are the content of this model. 
         * 
         * @param net - The specified Net
         */
        void setNet(Net* net);

        /**
         * Debug only
         * Sets a list of groupings as the content. 
         */ 
        void setGroupings(QList<Grouping*> groupingList);

        /**
         * Removes a specified amount of rows starting from a specified row.
         * 
         * @param row - The first row to remove. Row index must be valid.
         * @param count - The amount of rows to remove. Must be greater than 0. 
         *                All indices (row, ..., row+count-1) must be valid.
         * @returns true if the specified rows are removed successfully. Returns false for invalid parameters.
         */
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    private Q_SLOTS:
        // Slots to handle grouping or item changes that affect the grouping list or its data

        void handleGroupingRemoved(Grouping* grp);

        void handleGroupingNameChanged(Grouping* grp);

        void handleGroupingGateAssigned(Grouping* grp, u32 id);

        void handleGroupingGateRemoved(Grouping* grp, u32 id);

        void handleGroupingNetAssigned(Grouping* grp, u32 id);

        void handleGroupingNetRemoved(Grouping* grp, u32 id);

        void handleGroupingModuleAssigned(Grouping* grp, u32 id);

        void handleGroupingModuleRemoved(Grouping* grp, u32 id);

        void handleGroupingColorChanged(Grouping* grp);

    private:
        /**
         * Find the index of the sepcified Grouping in the GroupingTableEntry list. If the grouping can't be found return -1
         * @param grp - The specified Grouping
         * @returns the index of Grouping in the GroupingTableEntry list. (-1) if the grouping isn't found.
         */
        int getIndexOfGrouping(Grouping* grp) const;

        /// The List of models Groupings as GroupingTableEntries
        QList<GroupingTableEntry> mGroupings;

        /// The type of the oberserved item
        ItemType mItemType;

        /// The id of the observed item
        u32 mItemId;

    };
}
