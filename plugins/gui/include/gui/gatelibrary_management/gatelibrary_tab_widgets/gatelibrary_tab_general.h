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
#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_component_frame.h"
#include <QFormLayout>
#include <QLabel>
#include <QFrame>

namespace hal
{
    class GateLibraryLabel;
    class GatelibraryFrameFF;
    class GatelibraryFrameInit;
    class GatelibraryFrameState;
    class GatelibraryFrameLut;
    class GateLibraryFrameLatch;

    class GatelibraryFrameGeneral : public GatelibraryComponentFrame
    {
        Q_OBJECT
    public:
        GatelibraryFrameGeneral(QWidget* parent = nullptr);
        void update(GateType *gt) override;

    private:
        GateLibraryLabel* mNameLabel;
        GateLibraryLabel* mIdLabel;
        GateLibraryLabel* mPropertiesLabel;
    };

    class GatelibraryFrameBoolean : public GatelibraryComponentFrame
    {
        Q_OBJECT
    public:
        GatelibraryFrameBoolean(QWidget* parent = nullptr);
        void update(GateType *gt) override;
    };

    /**
     * Widget which shows general information of a given gate
     */
    class GateLibraryTabGeneral : public GateLibraryTabInterface
    {
        Q_OBJECT

    public:
        GateLibraryTabGeneral(QWidget* parent = nullptr);


        void update(GateType* gt) override;


    private:
        GatelibraryFrameGeneral* mGeneralFrame;
        GatelibraryFrameFF*      mFlipflopFrame;
        GatelibraryFrameState*   mStateFrame;
        GatelibraryFrameLut*     mLutFrame;
        GatelibraryFrameInit*    mInitFrame;
        GatelibraryFrameBoolean* mBooleanFrame;
        GateLibraryFrameLatch*   mLatchFrame;
    };

}
