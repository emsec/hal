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

#include "gui/selection_details_widget/gate_details_widget/parameter_table_model.h"

#include <QTableView>

namespace hal
{
    class Gate;

    /**
     * @ingroup utility_widgets-selection_details
     *
     * @brief A widget to display the typed parameters of a gate.
     *
     * Reads the parameters through the netlist API (Gate::get_parameters()) and shows them via the
     * ParameterTableModel. The gate to be shown is configured via setGate.
     */
    class ParameterTableWidget : public QTableView
    {
        Q_OBJECT

        public:
            /**
             * The constructor.
             *
             * @param parent - The widget's parent.
             */
            ParameterTableWidget(QWidget* parent = nullptr);

            /**
             * Sets the gate whose parameters are shown in the table.
             *
             * @param gate - The gate.
             */
            void setGate(Gate* gate);

    private:
            ParameterTableModel* mParameterTableModel;
    };
}
