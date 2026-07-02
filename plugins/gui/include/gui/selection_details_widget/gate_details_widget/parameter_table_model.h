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
#include <QVector>

namespace hal
{
    class Gate;

    /**
     * @ingroup utility_widgets-selection_details
     *
     * @brief A model to display the typed parameters of a gate.
     *
     * Shows one row per parameter with its name, type, bit-width, default value and current value.
     */
    class ParameterTableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent object.
         */
        ParameterTableModel(QObject* parent = nullptr);

        /**
         * Returns the amount of columns.
         *
         * @param parent - The parent model index.
         * @returns the amount of columns.
         */
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        /**
         * Returns the amount of rows in the table.
         *
         * @param parent - The parent model index.
         * @returns the amount of rows in the table.
         */
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        /**
         * Returns the data stored under the given role for a given index in the table model.
         *
         * @param index - The index in the table.
         * @param role - The access role.
         * @returns the data.
         */
        QVariant data(const QModelIndex& index, int role) const override;

        /**
         * Returns the header data fields.
         *
         * @param section - The section (column) index.
         * @param orientation - The orientation of the table.
         * @param role - The access role.
         * @returns the header data field at the given position.
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        /**
         * Fills the table with the parameters of the given gate and remembers the gate so that
         * edits can be written back to it.
         *
         * @param gate - The gate whose parameters are displayed.
         */
        void updateData(Gate* gate);

        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        Qt::ItemFlags flags(const QModelIndex &index) const override;

    private:
        struct ParameterRow
        {
            QString name;
            QString type;
            QString size;
            QString defaultValue;
            QString value;
        };

        QVector<ParameterRow> mRows;

        // Non-owning pointer to the gate currently displayed; the netlist owns the gate.
        Gate* mGate = nullptr;
    };
}
