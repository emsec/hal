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
#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"
#include "gui/pin_model/pin_item.h"


#include <QWizardPage>
#include <QGridLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>

namespace hal {
    class GateLibraryWizard;
    class StateWizardPage:public QWizardPage{
        friend class GateLibraryWizard;
        friend class FlipFlopWizardPage;
        friend class BoolWizardPage;
    public:
        Q_OBJECT
        StateWizardPage(QWidget* parent = nullptr);
        void setData(GateType* gate);
        void initializePage() override;
        bool isComplete() const override;
    private Q_SLOTS:
        void handleTextChanged(const QString &text);
        void handleNegTextChanged(const QString &text);
    private:
        QGridLayout* mLayout;

        QLineEdit* mStateIdentifier;
        QLineEdit* mNegStateIdentifier;

        QLabel* mLabStateIdentifier;
        QLabel* mLabNegStateIdentifier;

        GateLibraryWizard* mWizard;
        QList<PinItem*> mPinGroups;
        QValidator* mValidator;
    };
}
