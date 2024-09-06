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

#include <QFrame>
#include <QList>
#include <QCheckBox>
#include <QSpinBox>
#include <QStringList>
#include <QLabel>
#include <vector>
#include "hal_core/netlist/boolean_function.h"

namespace hal {

    class Net;

    class LogicEvaluatorHexSpinbox : public QSpinBox
    {
        Q_OBJECT
    protected:
        QValidator::State validate(QString &input, int &pos) const override;
        int valueFromText(const QString &text) const override;
        virtual QString textFromValue(int val) const override;
    public:
        LogicEvaluatorHexSpinbox(QWidget* parent = nullptr);
    };

    class LogicEvaluatorCheckBox : public QCheckBox
    {
        Q_OBJECT
        const Net* mNet;
    public:
        LogicEvaluatorCheckBox(const Net* n, QWidget* parent = nullptr);
        QPair<const Net*,BooleanFunction::Value> getValue() const;
        void setValue(const Net* n, BooleanFunction::Value val);
    };

    class LogicEvaluatorValue : public QWidget
    {
        Q_OBJECT
        QLabel* mLabel;
        LogicEvaluatorHexSpinbox* mSpinBox;

    private Q_SLOT:
        void handleSpinBoxValueChanged(int val);
    Q_SIGNALS:
        void valueChanged(int val);
    public:
        LogicEvaluatorValue(int nbits, QWidget* parent = nullptr);
        int value() const;
        void setValue(int val);
        bool isLabel() const { return mLabel!=nullptr; }
    };

    class LogicEvaluatorPingroup : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool output READ output WRITE setOutput);

        QList<LogicEvaluatorCheckBox*> mPinList;
        LogicEvaluatorValue* mGroupValue;
        bool mOutput;

        void constructor(const std::vector<const Net*>& nets, const QString& grpName = QString());
    Q_SIGNALS:
        void triggerRecalc();
    private Q_SLOT:
        void handleGroupValueChanged(int val);
        void handleCheckStateChanged(int state);
    public:
        LogicEvaluatorPingroup(const std::vector<const Net*>& nets, bool outp, const QString& grpName, QWidget* parent = nullptr);
        LogicEvaluatorPingroup(const Net*, bool outp, QWidget* parent = nullptr);
        bool output() const { return mOutput; }
        void setOutput(bool outp) { mOutput = outp; }
        int size() const { return mPinList.size(); }
        QPair<const Net*,BooleanFunction::Value> getValue(int index) const;
        void setValue(const Net* n, BooleanFunction::Value val);
    };
}
