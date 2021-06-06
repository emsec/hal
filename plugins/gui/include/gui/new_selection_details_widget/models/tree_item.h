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

#include <QList>



namespace hal
{

    /**
     * @brief (Future) Base class for all tree models related to the details widget.
     *
     * This class functions as a generic data container for all tree models. For this
     * purpose, it uses QVariants as its main type of storage for its columns. (Note: Perhaps add
     * additional data in form of a list or map (split it from "normal" displayed column data)
     */
    class TreeItem
    {
    // maybe add enum type for all possible scenarios? or use additional data with key to access type
    // and handle type handling in model...e.g.: item->getAddData("type")(structure, more generalization,...)
    public:
        /**
         * The constructor.
         *
         * @param parent - The item's parent.
         */
        TreeItem(TreeItem* parent = nullptr);

        /**
         * Second constructor to immediately assign column data.
         *
         * @param parent - The item's parent.
         * @param columnData - The item's data.
         */
        TreeItem(TreeItem* parent = nullptr, QList<QVariant> columnData  = QList<QVariant>());

        /**
         * Get the data of a specific column (most in the form of a string).
         *
         * @param column - The requested column.
         * @return The data if within the column count. Empty QVariant otherwise.
         */
        QVariant getData(int column);

        /**
         * Sets the data for all columns.
         *
         * @param data - Each entry in the list represents one column.
         */
        void setData(QList<QVariant> data);

        /**
         * Sets the data for a specified column. The index must be within
         * already existing boundaries (for example, add dummy data beforehand).
         *
         * @param index - The column to set the new data.
         * @param data - The new column data.
         */
        void setDataAtIndex(int index, QVariant data);

        /**
         * Appends a new column to the item.
         *
         * @param data - The data of the new column.
         */
        void appendData(QVariant data);

        /**
         * Get the item's parent.
         *
         * @return The parent.
         */
        TreeItem* getParent();

        /**
         * Sets the item's parent.
         *
         * @param parent - The parent.
         */
        void setParent(TreeItem* parent);

        /**
         * Get the child of a specific row.
         *
         * @param row - The requested row.
         * @return The child if within bounds. Nullptr otherwise.
         */
        TreeItem* getChild(int row);

        /**
         * Appends a child.
         *
         * @param child - The child to append.
         */
        void appendChild(TreeItem* child);

        /**
         * Get the number of children.
         *
         * @return The number of children.
         */
        int getChildCount();

    private:
        TreeItem* mParent;
        QList<TreeItem*> mChildren;
        QList<QVariant> mData;

        // experimental, additional data (for anything)
        //QMap<QString, QVariant> mAdditionalData;
        //QList<QVariant> mAdditionalData;

    };
}
