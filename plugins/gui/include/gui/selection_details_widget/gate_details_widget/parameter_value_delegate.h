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

#include "hal_core/netlist/parameter.h"

#include <QModelIndex>
#include <QItemDelegate>

namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     *
     * @brief Editor delegate for the "Value" column of the parameter table.
     *
     * Picks the editor widget to show for a given row based on the parameter's
     * Parameter::Type: a combo box for Boolean / Enum, a text field for the
     * remaining types. The chosen value is written back through the model's
     * setData (which validates and forwards it to the gate).
     */
    class ParameterValueDelegate : public QItemDelegate
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent object.
         */
        explicit ParameterValueDelegate(QObject* parent = nullptr);

        /**
         * Creates the editor widget appropriate for the row's parameter type.
         *
         * @param parent - The parent widget the editor is created on.
         * @param option - The style options for the item.
         * @param index - The index being edited (column 4, the value column).
         * @returns The editor widget, or nullptr if the cell is not editable.
         */
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        /**
         * Reads the value from the editor and writes it back through the model.
         *
         * @param editor - The editor returned by createEditor.
         * @param model - The model to write the new value into.
         * @param index - The index being edited.
         */
        void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

        /**
         * Sizes the editor to fill the cell.
         *
         * @param editor - The editor returned by createEditor.
         * @param option - The style options carrying the cell rectangle.
         * @param index - The index being edited.
         */
        void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    private:
        /**
         * Reads the parameter type for the given row from the model.
         *
         * The type drives which editor createEditor builds. The model must expose
         * the type for the row (e.g. via a custom data role); see implementation.
         *
         * @param index - Any index in the row being edited.
         * @returns The parameter type of that row.
         */
        Parameter::Type parameterType(const QModelIndex& index) const;
    };
}    // namespace hal
