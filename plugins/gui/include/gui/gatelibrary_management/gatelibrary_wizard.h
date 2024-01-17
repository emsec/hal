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
#include "hal_core/netlist/gate_library/gate_type.h"

#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/pins_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/flipflop_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/bool_wizardpage.h"

#include <QWizard>
#include <QWizardPage>

namespace hal
{
    class GateLibraryWizard : public QWizard
    {
    public:
        enum PAGE
        {
            GeneralInfo = 0,
            Pin = 1,
            FlipFlop = 2,
            BooleanFunction = 3,
            None = -1
        };

        GateLibraryWizard(const GateLibrary* gateLibrary, GateType* gateType, QWidget* parent = nullptr);
        GateLibraryWizard(const GateLibrary* gateLibrary, QWidget* parent = nullptr);

        void editGate(GateType* gt);
        void addGate();
        void setData(GateLibrary* gateLibrary, GateType* gateType);
        QStringList getProperties();
        void accept() override;

        int getNextPageId(PAGE page);
        void setPageOrder();
    private:
        const GateLibrary* mGateLibrary;
        GateType* mGateType;
        QMap<PAGE, PAGE>  mPageLookupTable;
        GeneralInfoWizardPage* generalInfoPage;
        PinsWizardPage* pinsPage;
        FlipFlopWizardPage* ffPage;
        BoolWizardPage* boolPage;

        QString mName;
        QStringList mProperties;
    };
}
