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

#include <QObject>
#include <QString>
#include <QVariant>

namespace hal
{
    class SettingsWidget;

    class SettingsItem : public QObject
    {
        Q_OBJECT

    public:
        SettingsItem(QObject* parent = nullptr) : QObject(parent) {}
        virtual ~ SettingsItem() {}

        virtual QVariant value() const = 0;
        virtual QVariant defaultValue() const = 0;
        virtual void setValue(const QVariant& v) = 0;
        virtual  SettingsWidget* editWidget(QWidget* parent = nullptr) = 0;

        virtual bool isDefaultValue() const
        {
            return value() == defaultValue();
        }

        virtual QVariant persistToSettings() const
        {
            return value();
        }

        virtual void restoreFromSettings(const QVariant& val)
        {
            setValue(val);
        }

        virtual bool isGlobal() const
        {
            return mIsGlobal;
        }

        virtual QString label() const
        {
            return mLabel;
        }

        virtual QString description() const
        {
            return mDescription;
        }

        virtual QString tag() const
        {
            return mTag;
        }

        virtual QString category() const
        {
            return mCategory;
        }

    Q_SIGNALS:
        void valueChanged();

    protected:
        bool mIsGlobal;
        QString mLabel;
        QString mDescription;
        QString mTag;
        QString mCategory;
    };
}
