//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "gui/validator/stacked_validator.h"

#include <QKeySequenceEdit>
#include <QEvent>
#include <QColor>

namespace hal
{
    class SettingsItemKeybind;

    class KeybindEdit : public QKeySequenceEdit
    {
    Q_OBJECT
    Q_PROPERTY(bool hasGrab READ hasGrab WRITE setHasGrab)
    Q_PROPERTY(bool validated READ validated WRITE setValidated)

    public:
        KeybindEdit(QWidget* parent = nullptr);
        void setHasGrab(bool isgrab);
        void setValidated(bool valid);
        bool hasGrab();
        bool validated();
        bool doValidate();
        void load(const QKeySequence& seq, SettingsItemKeybind* item);

    Q_SIGNALS:
        void editRejected(QString errmsg);
        void editAccepted();

    private Q_SLOTS:
        void restoreOldSequence();

    protected:
        bool event(QEvent* e) override;

    private:
        SettingsItemKeybind* mItem;
        QKeySequence mOldSequence;
        bool mSkipValidate;
        bool mValidated;
        bool mGrab;
    };
}
