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

#include <QObject>
#include <QString>
#include <QVariant>

namespace hal
{
    class SettingsWidget;

    /**
     * @ingroup settings
     * @brief The interface for the logical part of a setting.
     *
     * The SettingsItem class provides an interface and commonly shared function
     * for all kinds of specific settings. It is the logical part of a setting in
     * comparison to the SettingsWidget, which represents the corresponding visual
     * part. It -more precise the specific settings- connects to the SettingsManager.
     */
    class SettingsItem : public QObject
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param parent - The settings parent.
         */
        SettingsItem(QObject* parent = nullptr) : QObject(parent) {}

        /**
         * The destructor.
         */
        virtual ~ SettingsItem() {}

        /**
         * Get the current value.
         *
         * @return The current value.
         */
        virtual QVariant value() const = 0;

        /**
         * Get the default value.
         *
         * @return The default value.
         */
        virtual QVariant defaultValue() const = 0;

        /**
         * Sets the value of the setting.
         *
         * @param v - The value to set.
         */
        virtual void setValue(const QVariant& v) = 0;

        /**
         * Sets the default value of the setting.
         *
         * @param dv - The default value.
         */
        virtual void setDefaultValue(const QVariant& dv) = 0;

        /**
         * Get a corresponding SettingsWidget which holds a reference to the
         * settings item.
         *
         * @param parent - The widget's soon to be parent.
         * @return A SettingsWidget able to modify the setting.
         */
        virtual  SettingsWidget* editWidget(QWidget* parent = nullptr) = 0;

        /**
         * Compares the default value to the current value.
         *
         * @return True if they are the same. False otherwise.
         */
        virtual bool isDefaultValue() const
        {
            return value() == defaultValue();
        }

        /**
         * Get the value of the item. See value().
         *
         * @return The current value.
         */
        virtual QVariant persistToSettings() const
        {
            return value();
        }

        /**
         * Sets the value of the settings to the given value.
         *
         * @param val - The value to set.
         */
        virtual void restoreFromSettings(const QVariant& val)
        {
            setValue(val);
        }

        /**
         * Checks if the setting is a global setting.
         *
         * @return True if it is, False otherwise.
         */
        virtual bool isGlobal() const
        {
            return mIsGlobal;
        }

        /**
         * Get the settings label.
         *
         * @return The label.
         */
        virtual QString label() const
        {
            return mLabel;
        }

        /**
         * Get the setting's description.
         *
         * @return The setting's description.
         */
        virtual QString description() const
        {
            return mDescription;
        }

        /**
         * Get the setting's tag (key).
         *
         * @return The tag (key).
         */
        virtual QString tag() const
        {
            return mTag;
        }

        /**
         * Get the category this setting belongs to.
         *
         * @return The category.
         */
        virtual QString category() const
        {
            return mCategory;
        }

    Q_SIGNALS:

        /**
         * Q_SIGNAL that should be commited when the value changes.
         */
        void valueChanged();

    protected:
        bool mIsGlobal;
        QString mLabel;
        QString mDescription;
        QString mTag;
        QString mCategory;
    };
}
