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
#include <QFont>
#include <QColor>

#include "hal_core/defines.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"

namespace hal {

    /**
     * @ingroup utility_widgets-selection_details
     *
     * @brief A model to display the data of a DataContainer
     *
     */
    class DataTableModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:

        /**
         * Helper enum that can be used to access the value of a data entry.
         */
        enum propertyType{category = 0, key = 1, type = 2, value = 3};
        /**
         * Used to store one data entry (i.e. the data in one data field).
         */
        struct DataEntry
        {
            QString category; /// The data's category
            QString key; /// The data's key
            QString dataType; // The data type
            QString value; // The value of the data 
            QString getPropertyValueByPropType(propertyType prop)//helper function to generically acces a propertys value
            {
                QString val =  (prop == 0) ? category : ( (prop == 1) ? key : ( (prop == 2) ? dataType : value ) );
                return val;
            }
        };

        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        DataTableModel(QObject* parent=nullptr);

        /**
         * Returns the amount of columns.
         *
         * @param parent - Then parent model index
         * @returns the amount of columns.
         */
        int columnCount(const QModelIndex &parent=QModelIndex()) const override;

        /**
         * Returns the amount of rows in the table.
         *
         * @param parent - The parent model index
         * @returns the amount of rows in the table.
         */
        int rowCount(const QModelIndex &parent=QModelIndex()) const override;

        /**
         * Returns the data stored under the given role for a given index in the table model. <br>
         * All values are aligned centered. The last column is the rows output,
         * in the other columns are the input values.
         *
         * @param index - The index in the table
         * @param role - The access role
         * @returns the data
         */
        QVariant data(const QModelIndex &index, int role) const override;

        /**
         * Returns the header data fields.
         *
         * @param section - The section (column) index
         * @param orientation - The orientation of the table
         * @param role - The access role
         * @returns the header data field at the given position
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        /**
         * Sets the role data for the item at index to value. Currently unused.
         *
         * @param index - The model index
         * @param value - The value to set
         * @param role - The access role
         * @returns <b>true</b> on success
         */
        bool setData(const QModelIndex &index, const QVariant &value, int role) override;

        /**
         * Accesses the DataEntry at the specified row. 
         * 
         * @param row - The specified row
         * @returns the DataEntry at the specified row
         */
        DataEntry getEntryAtRow(int row) const;

        /**
         * Fills the table with the given data in the form of key-value pairs.
         *
         * @param dc - The data in the form of key - vlaue pairs.
         */
        void updateData(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& dc);

    private:
        struct RowStyle
        {
            QFont keyFont;
            QColor valueColor;
            QString valueString;
            QString keyToolTip;
            QString valueToolTip;
        };

        RowStyle getRowStyleByEntry(const DataEntry& entry, int rowIdx) const;

        // Map: [category, key] -> row style of the respective entry
        QMap<QPair<QString, QString>, RowStyle> mEntryToRowStyle;
        QList<DataEntry> mDataEntries;
    };
}
