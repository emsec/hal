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
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"

#include "hal_core/defines.h"

namespace hal {

    class BooleanFunctionTableModel : public QAbstractTableModel
    {
    Q_OBJECT
    private:

        struct BooleanFunctionTableEntry
        {
            QString functionName; // e.g. "O" or "clock"
            QString functionString; // e.g. "I0 & !I1"
        };

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        BooleanFunctionTableModel(QObject* parent=nullptr);

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


        void setBooleanFunctionList(const QMap<QString, BooleanFunction>& latchOrFFFunctions,
                                    const QMap<QString, BooleanFunction>& customFunctions,
                                    std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> cPBehaviour);

        void setBooleanFunctionList(const QMap<QString, BooleanFunction>& latchOrFFFunctions,
                                    const QMap<QString, BooleanFunction>& customFunctions);

        void setSingleBooleanFunction(QString functionName, BooleanFunction booleanFunction);

    private:
        QList<BooleanFunctionTableEntry> getEntryListFromMap(const QMap<QString, BooleanFunction>& map);
        BooleanFunctionTableEntry getEntryFromCPBehaviour(std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> cPBehaviour);

        QList<BooleanFunctionTableEntry> mLatchOrFFFunctions;
        QList<BooleanFunctionTableEntry> mCustomFunctions;
        // maybe:
        bool mIncludeCSBehaviour;
        BooleanFunctionTableEntry mCSBehaviour;


    };
}
