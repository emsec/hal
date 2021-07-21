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

        bool mDisableEvents;

        QList<GroupingTableEntry> mGroupings;
        QString mAboutToRename;

        ItemType mItemType;
        u32 mItemId;

        //static QString generateUniqueName(const QString& suggestion, const QSet<QString>& existingNames);
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
        bool setData(const QModelIndex &index, const QVariant &value, int role) override;

        /**
          * Gets the table entry that contains the grouping at a given row index.
          *
          * @param irow - The row index
          * @returns the grouping table entry
          */
        GroupingTableEntry groupingAt(int irow) const { return mGroupings.at(irow); }

        void setGate(Gate* gate);
        void setModule(Module* module);
        void setNet(Net* net);

        /**
         * Remove the row at index row.
         *
         * @param row - The row index
         * @param count - Unused and ignored (always one row is deleted at a time)
         * @param parent - Unused and ingnored
         * @returns <b>true</b> on success
         */
        //bool removeRows(int row, int count=1, const QModelIndex &parent=QModelIndex()) override;

        /**
         * Returns <b>true</b> if <i>input</i> is a unique and valid grouping name. Returns <b>false</b> otherwise.
         *
         * @param input - A grouping name
         * @returns <b>true</b> if <i>input</i> is a unique and valid grouping name.
         *          Returns <b>false</b> otherwise
         */
        //bool validate(const QString &input);

        /**
         * This function is used to prevent the 'validate'-function to return <b>false</b> while renaming a grouping to
         * its old name. Therefore the old name of the grouping will always considered as valid. <br>
         * To achieve this one has to call this function to store the old name.
         *
         * @param oldName - The old name of the grouping
         */
        //void setAboutToRename(const QString& oldName) { mAboutToRename = oldName.trimmed(); }

         

        /**
         * Adds a grouping to the table. The grouping is created with a unique default name (e.g. 'grouping1').
         *
         * @returns the created grouping
         */
        //Grouping* addDefaultEntry();

        /**
         * Searches for a grouping with the given name in the grouping table.
         *
         * @param name - The name to search for
         * @returns the grouping if found. Returns <i>nullptr</i> otherwise.
         */
        //Grouping* groupingByName(const QString& name) const;

        /**
         * Gets the color of an item based on its grouping. If the item has no grouping an invalid color (QColor()) is
         * returned.
         *
         * @param itemType - The type of the item (i.e. Module, Gate or Net)
         * @param itemId - The id of the item
         * @returns the color of the grouping of the item. Returns an invalid color if the item has no grouping.
         */
        //QColor colorForItem(ItemType itemType, u32 itemId) const;

      /**
         * Renames the grouping at the given table row of the groupings table.
         *
         * @param id - The id of the grouping to rename
         * @param groupingName - The new name
         * @returns The old name.
         */
        //QString renameGrouping(u32 id, const QString& groupingName);

        /**
         * Gets a list of all currently used grouping names.
         *
         * @returns a string list of used grouping names
         */
        //QStringList groupingNames() const;

        

    public Q_SLOTS:
        /**
         * Q_SLOT to handle that a grouping was removed.
         *
         * @param grp - The removed grouping.
         */
        //void deleteGroupingEvent(Grouping* grp);

        /**
         * Changes the color of the grouping with the specified id to the specified color.
         *
         * @param id - The grouping id
         * @param groupingColor - The new color
         * @returns the color BEFORE the recoloring. If the grouping id is unknown an empty color (QColor()) is returned
         */
        //QColor recolorGrouping(u32 id, const QColor& groupingColor);

        //void recoloredGroupingEvent(u32 id, const QColor& groupingColor);

        /**
         * Q_SLOT to handle that a grouping has been created.
         *
         * @param grp - The created grouping
         */
        //void createGroupingEvent(Grouping *grp);

        /**
         * Q_SLOT to handle that a grouping has been renamed.
         *
         * @param grp
         */
        //void groupingNameChangedEvent(Grouping *grp); // TODO

    Q_SIGNALS:
        /**
         * Q_SIGNAL to notify that the color of a grouping has been changed. Emitted by GroupingTableModel::setData if
         * the color field was modified.
         *
         * @param grp - The grouping which color has been changed.
         */
        //void groupingColorChanged(Grouping* grp);

        /**
         * Q_SIGNAL to notify that the grouping table entry at the rearmost position
         * (i.e. the entry with the highest row index) has been deleted.
         */
        //void lastEntryDeleted();

        /**
         * Q_SIGNAL to notify that a new grouping table entry has been added to the table.
         *
         * @param index - The index of the new entry
         */
        //void newEntryAdded(QModelIndex& index);

    private:
        //QColor nextColor() const;
    };
}
