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

#include <QWizardPage>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVector>

#include "hal_core/defines.h"

namespace hal {
    class LUTWizardPage : public QWizardPage {
        Q_OBJECT
        friend class GateLibraryWizard;
    public:
        struct OutputPinEntry {
            QString pinName;
            QString initIdentifier;
            u32     bitOffset;
            u32     bitCount;
        };

        LUTWizardPage(QWidget* parent = nullptr);
        void setData(GateType* gate);
        void initializePage() override;

        QVector<OutputPinEntry> getOutputPinConfigs() const;

    private Q_SLOTS:
        void addRow();
        void removeSelectedRow();

    private:
        void addTableRow(const QString& pinName, const QString& initId,
                         u32 bitOffset, u32 bitCount,
                         const QStringList& availablePins);
        void updateDropdowns(const QStringList& pins);
        QStringList getOutputPinsFromWizard() const;

        QGridLayout*  mLayout;
        QCheckBox*    mAscending;
        QLabel*       mLabAscending;
        QTableWidget* mPinConfigTable;
        QPushButton*  mAddBtn;
        QPushButton*  mRemoveBtn;

        struct SavedConfig {
            std::string pinName;
            std::string initIdentifier;
            u32 bitOffset;
            u32 bitCount;
        };
        std::vector<SavedConfig> mSavedConfigs;
        bool mTableInitialized = false;
    };
}
