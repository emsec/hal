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


    class GroupingsOfItemModel : public QAbstractTableModel
    {
        Q_OBJECT

        //bool mDisableEvents;

        QList<GroupingTableEntry> mGroupings;
        QString mAboutToRename;

        ItemType mItemType;
        u32 mItemId;

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        GroupingsOfItemModel(QObject* parent=nullptr);

        /**
         * Returns the amount of columns. Is always 3 (Grouping, ID, Color)
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

        void setGate(Gate* gate);
        void setModule(Module* module);
        void setNet(Net* net);

        // Debug Only
        void setGroupings(QList<Grouping*> groupingList);

        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    private Q_SLOTS:

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

    };
}
