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

#include "gui/settings/settings_widgets/settings_widget.h"


#include "gui/keybind_edit/keybind_edit.h"

#include <QCheckBox>
#include <QStringList>
#include <QLabel>

class QTimer;

namespace hal
{
    class SettingsItemKeybind;

    /**
     * @ingroup settings
     * @brief A setting to configure keybinds.
     */
    class SettingsWidgetKeybind : public SettingsWidget
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param item - The underlying keybind item.
         * @param parent - The widget's parent.
         */
        SettingsWidgetKeybind(SettingsItemKeybind* item, QWidget* parent = nullptr);

        /**
         * Overwritten SettingsWidget function.
         *
         * @param value - The value (keysquence) to load into the widget.
         */
        virtual void load(const QVariant& value) override;

        /**
         * Overwritten SettingsWidget function.
         *
         * @return The currently visual configured keybind value.
         */
        virtual QVariant value() override;

        /**
         * Overwritten SettingsWidget function.
         * Removes the current value of the underlying KeybindEdit.
         */
        virtual void clearEditor() override;

        /**
         * Overwritten SettingsWidget function.
         */
        void handleRevertModification() override;

        /**
         * Overwritten SettingsWidget function.
         */
        void handleSetDefaultValue(bool setAll = false) override;

    private:
        KeybindEdit* mKeybindEdit;
        QLabel* mErrorMessage;

        bool isKeybindUsed(const QVariant& testValue);

    private Q_SLOTS:
        void clearErrorMessage();
        void onKeybindEditAccepted();
        void onKeybindEditRejected(const QString& errMsg);
    };
}
