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

#include <QSpinBox>

namespace hal
{
    class SettingsItemSpinbox;

    /**
     * @ingroup settings
     * @brief A SettingsWidget representing a spinbox.
     */
    class SettingsWidgetSpinbox : public SettingsWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param item - The underlying settings item.
         * @param parent - The widget's parent.
         */
        SettingsWidgetSpinbox(SettingsItemSpinbox* item, QWidget* parent = nullptr);

        /**
         * Overwritten SettingsWidget function.
         *
         * @param value - The value to load into the widget.
         */
        virtual void load(const QVariant& value) override;

        /**
         * Overwritten SettingsWidget function.
         *
         * @return The visual value of the spinbox.
         */
        virtual QVariant value() override;

    private:
        void on_spinbox_value_changed(int value);

        QSpinBox* mSpinbox;
    };
}
