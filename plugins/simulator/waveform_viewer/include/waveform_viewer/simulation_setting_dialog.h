//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include "netlist_simulator_controller/simulation_settings.h"
#include "gui/main_window/color_selection.h"

namespace hal {

    class SimulationSettings;

    class SimulationSettingColorTab : public QWidget
    {
        Q_OBJECT
        ColorSelection* mActivateColorDialog[SimulationSettings::MaxColorSetting];

    public:
        SimulationSettingColorTab(SimulationSettings* settings, QWidget* parent = nullptr);
        QString colorSetting(int inx) const;
    };

    class SimulationSettingPropertiesTab : public QTableWidget
    {
        Q_OBJECT
    private Q_SLOTS:
        void handleCellChanged(int irow, int icolumn);
    public:
        SimulationSettingPropertiesTab(SimulationSettings* settings, QWidget* parent = nullptr);
        QMap<QString,QString> engineProperties() const;
    };

    class SimulationSettingGlobalTab : public QWidget
    {
        Q_OBJECT
        QSpinBox* mMaxSizeLoadable;
        QSpinBox* mMaxSizeEditor;
        QCheckBox* mCustomBaseDicectory;
        QLineEdit* mEditBaseDirectory;
    private Q_SLOTS:
        void customBaseDirectoryToggled(bool on);
    public:
        SimulationSettingGlobalTab(SimulationSettings* settings, QWidget* parent = nullptr);
        int maxSizeLoadable() const { return mMaxSizeLoadable->value(); }
        int maxSizeEditor() const { return mMaxSizeEditor->value(); }
        QString baseDirectory() const { return mEditBaseDirectory->text(); }
        bool isCustomBaseDirectory() const { return mCustomBaseDicectory->isChecked(); }
    };

    class SimulationSettingDialog : public QDialog
    {
        Q_OBJECT
        SimulationSettings* mSettings;
        QTabWidget* mTabWidget;
        QDialogButtonBox* mButtonBox;
    private Q_SLOTS:
        void accept() override;
    public:
        SimulationSettingDialog(SimulationSettings* settings, QWidget* parent = nullptr);
    };
}
