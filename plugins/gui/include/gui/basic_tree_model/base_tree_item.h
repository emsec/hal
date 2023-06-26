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

#include <QList>
#include <QVariant>


namespace hal
{

    /**
     * @brief (Future) Base class for all tree models related to the details widget.
     *
     * This class functions as a generic data container for all tree models. For this
     * purpose, it uses QVariants as its main type of storage for its columns. (Note: Perhaps add
     * additional data in form of a list or map (split it from "normal" displayed column data)
     */
    class BaseTreeItem
    {
    // maybe add enum type for all possible scenarios? or use additional data with key to access type
    // and handle type handling in model...e.g.: item->getAddData("type")(structure, more generalization,...)

    private:
        /**
         * Copy constructor. Copies the item's data, not the parent/children.
         *
         * @param item - The item to copy.
         */
        BaseTreeItem(const BaseTreeItem &item);

    public:
        /**
         * The constructor.
         */
        BaseTreeItem();

        /**
         * Second constructor to immediately assign column data.
         *
         * @param columnData - The item's data.
         */
        BaseTreeItem(QList<QVariant> columnData);

        /**
          * The destructor.
          */
        virtual ~BaseTreeItem();

        /**
         * Get the data of a specific column (most in the form of a string).
         *
         * @param column - The requested column.
         * @return The data if within the column count. Empty QVariant otherwise.
         */
        virtual QVariant getData(int column) const = 0;

        /**
         * Sets the data for all columns.
         *
         * @param data - Each entry in the list represents one column.
         */
        virtual void setData(QList<QVariant> data) = 0;

        /**
         * Sets the data for a specified column. The index must be within
         * already existing boundaries (for example, add dummy data beforehand).
         *
         * @param index - The column to set the new data.
         * @param data - The new column data.
         */
        virtual void setDataAtIndex(int index, QVariant& data);

        /**
         * Appends a new column to the item.
         *
         * @param data - The data of the new column.
         */
        virtual void appendData(QVariant data);

        /**
         * Get the item's parent.
         *
         * @return The parent.
         */
        virtual BaseTreeItem* getParent() const;

        /**
         * Sets the item's parent.
         *
         * @param parent - The parent.
         */
        virtual void setParent(BaseTreeItem* parent);

        /**
         * Get the child of a specific row.
         *
         * @param row - The requested row.
         * @return The child if within bounds. Nullptr otherwise.
         */
        virtual BaseTreeItem* getChild(int row) const;

        /**
         * Get the list of all children.
         *
         * @return The list of children.
         */
        virtual QList<BaseTreeItem*> getChildren() const;

        /**
         * Appends a child.
         *
         * @param child - The child to append.
         */
        virtual void appendChild(BaseTreeItem* child);

        /**
         * Inserts a child at the given index. If the index exceeds the amount
         * of children, the new child is appended.
         *
         * @param index - The position at which to insert.
         * @param child - The child to insert.
         */
        virtual void insertChild(int index, BaseTreeItem* child);

        /**
         * Removes the child at the given row and returns it.
         *
         * @param row - The row from which to remove the child.
         * @return The removed child. Nullptr if row was out of bounds.
         */
        virtual BaseTreeItem* removeChildAtPos(int row);

        /**
         * Removes the given item and returns True if removing was successful
         * or False if the given item was no child.
         *
         * @param child - The child to remove.
         * @return True on success, False otherwise.
         */
        virtual bool removeChild(BaseTreeItem* child);

        /**
         * Get the number of children.
         *
         * @return The number of children.
         */
        virtual int getChildCount() const;

        /**
         * Get the number of currently stored column data.
         *
         * @return The column count.
         */
        virtual int getColumnCount() const = 0;

        /**
         * Convenience method to get the row for a given item.
         * If the item is not a child of this item, -1 is returned.
         *
         * @param child - The child for which the row is requested.
         * @return The row if the item is a child, -1 otherwise.
         */
        virtual int getRowForChild(BaseTreeItem* child) const;

        /**
         * Convenience method to get the row of this item within
         * the parent's list. If the item has no parent, -1 is returned.
         *
         * @return The row of this item if it has a parent, -1 otherwise.
         */
        virtual int getOwnRow();

        /**
         * Stores additional data. Can be accessed by getAdditionalData.
         * (For example, a menu or color)
         *
         * @param key - The key to store the data under.
         * @param data - The actual data to store.
         */
        virtual void setAdditionalData(QString key, QVariant data);

        /**
         * Retrieve the data stored under the given key.
         *
         * @param key - The key for the requested data.
         * @return The data if something was stored under the key, empty QVariant otherwise.
         */
        virtual QVariant getAdditionalData(QString key) const;

    private:
        BaseTreeItem* mParent;
        QList<BaseTreeItem*> mChildren;

        // experimental, additional data (for anything)
        QMap<QString, QVariant> mAdditionalData;
        //QList<QVariant> mAdditionalData;

    };

    /**
     * Since the BaseTreeItem class is pure virtual it cannot be instanciated for
     * root tree item.
     *
     * RootTreeItem class also provides the header labels.
     */
    class RootTreeItem : public BaseTreeItem
    {
        QStringList mHeaderLabels;
    public:
        RootTreeItem(const QStringList& labels) : mHeaderLabels(labels) {;}

        /**
         * Get header label for section.
         * @param column The section of the header.
         * @return The label for the header.
         */
        QVariant getData(int column) const override;

        /**
         * Set header label to new value. If element in list does not exist it gets created.
         * @param column The section of the header.
         * @param data The string value.
         */
        void setData(QList<QVariant> data) override;

        /**
         * Sets the data for a specified column. The index must be within
         * already existing boundaries (for example, add dummy data beforehand).
         *
         * @param index - The column to set the new data.
         * @param data - The new column data.
         */
        void setDataAtIndex(int index, QVariant& data);

        /**
         * Appends a new column to the item.
         *
         * @param data - The data of the new column.
         */
        void appendData(QVariant data);

        /**
         * Get number of sections for which header label exist.
         * @return The number of sections.
         */
        int getColumnCount() const override { return mHeaderLabels.size(); }
    };
}
