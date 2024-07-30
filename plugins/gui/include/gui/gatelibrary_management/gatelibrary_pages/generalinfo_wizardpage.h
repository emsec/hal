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

#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "hal_core/netlist/gate_library/gate_library.h"

#include <QWizardPage>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QIcon>

namespace hal {
    class GeneralInfoWizardPage:public QWizardPage{
        Q_OBJECT

        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString enabledIconStyle READ enabledIconStyle WRITE setEnabledIconStyle)
        Q_PROPERTY(QString leftArrowIconPath READ leftArrowIconPath WRITE setLeftArrowIconPath)
        Q_PROPERTY(QString rightArrowIconPath READ rightArrowIconPath WRITE setRightArrowIconPath)
    public:
        GeneralInfoWizardPage(const GateLibrary* gt, QWidget* parent = nullptr);
        void setData(QString name, QStringList properties);
        QString getName();
        QStringList getProperties();
        void setMode(bool edit);
        bool isEdit();
        bool validatePage() override;

        QString disabledIconStyle()  const { return mDisabledIconStyle; }
        QString enabledIconStyle()   const { return mEnabledIconStyle; }
        QString leftArrowIconPath()  const { return mLeftArrowIconPath; }
        QString rightArrowIconPath() const { return mRightArrowIconPath; }

        void setDisabledIconStyle (const QString& s) { mDisabledIconStyle = s; }
        void setEnabledIconStyle  (const QString& s) { mEnabledIconStyle = s; }
        void setLeftArrowIconPath (const QString& p) { mLeftArrowIconPath = p; }
        void setRightArrowIconPath(const QString& p) { mRightArrowIconPath = p; }

    public Q_SLOTS:
        void addProperty();
        void deleteProperty();
    private:
        QWizard* mWizard;
        QGridLayout* mLayout;
        QLabel* mLabelName;
        QLabel* mLabelAddProperty;

        QLineEdit* mName;
        QListWidget* mPropertiesSelected;
        QListWidget* mPropertiesAvailable;

        QComboBox* mAddProperty;

        QPushButton* mDelBtn;
        QPushButton* mAddBtn;

        QIcon mLeftArrowIcon;
        QIcon mRightArrowIcon;

        const GateLibrary* mGateLibrary;
        bool mIsEdit;
        QString gateInit;

        QString mDisabledIconStyle;
        QString mEnabledIconStyle;
        QString mLeftArrowIconPath;
        QString mRightArrowIconPath;
        //QStringList mPropertiesList;
    };
}
