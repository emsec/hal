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
#include "gui/gatelibrary_management/gatelibrary_pages/latch_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/lut_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/init_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/ram_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/ram_port_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_pages/state_wizardpage.h"


#include <QWizard>
#include <QWizardPage>

namespace hal
{
    class GateLibraryWizard : public QWizard
    {
        Q_OBJECT

        friend class GeneralInfoWizardPage;
        friend class PinsWizardPage;
        friend class FlipFlopWizardPage;
        friend class BoolWizardPage;
        friend class LatchWizardPage;

        friend class RAMPortWizardPage;
        friend class StateWizardPage;
        friend class InitWizardPage;
    public:
        enum PAGE
        {
            GeneralInfo,
            Pin,
            FlipFlop,
            Latch,
            LUT,
            RAM,
            RAMPort,
            Init,
            State,
            BoolFunc
        };

        GateLibraryWizard(GateLibrary* gateLibrary, GateType* gateType = nullptr, QWidget* parent = nullptr);

        QList<PinItem*> getPingroups();
        std::unique_ptr<GateTypeComponent> setComponents();
        GateType* getRecentCreatedGate();
        void accept() override;
        int nextId() const override;
    Q_SIGNALS:
        void triggerUnsavedChanges();
    private Q_SLOTS:
        void handleWasEdited();
    private:
        GateLibrary* mGateLibrary;
        GateType* mGateType;
        GeneralInfoWizardPage* generalInfoPage;
        PinsWizardPage* pinsPage;
        FlipFlopWizardPage* ffPage;
        LatchWizardPage* latchPage;
        LUTWizardPage* lutPage;
        InitWizardPage* initPage;
        RAMWizardPage* ramPage;
        RAMPortWizardPage* ramportPage;
        StateWizardPage* statePage;
        BoolWizardPage* boolPage;

        PinModel* mPinModel;
        GateType* mNewGateType;
        bool mWasEdited;
        bool mEditMode;
        QString mTitle;
    };
}
