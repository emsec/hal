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

#include "gatelibrary_tab_interface.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate.h"

#include <QFormLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>

namespace hal
{
    /**
     * Widget which shows information of the boolean functions of a gate
     */
    class GateLibraryTabTruthTable : public GateLibraryTabInterface
    {
        Q_OBJECT

    public:
        GateLibraryTabTruthTable(QWidget* parent = nullptr);


        void update(GateType* gate) override;
        int getRowNumber(GateType* gate);
        int getColumnNumber(GateType* gate);


    private:

        /*QFormLayout* mFormLayout;

        QLabel* mPlaceholderLabel1;
        QLabel* mPlaceholderLabel2;
        QLabel* mPlaceholderLabel3;
        QLabel* mPlaceholderLabel4;
        QLabel* mPlaceholderLabel5;

        QLabel* mPlaceholderPropertyLabel1;
        QLabel* mPlaceholderPropertyLabel2;
        QLabel* mPlaceholderPropertyLabel3;
        QLabel* mPlaceholderPropertyLabel4;
        QLabel* mPlaceholderPropertyLabel5;*/

        QGridLayout* mLayout;
        QTableWidget* mTableWidget;
        QHeaderView* mHeaderView;
        QLabel* mDisclaimer;
    };

}
