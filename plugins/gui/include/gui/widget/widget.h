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

#include <QWidget>

namespace hal
{
    /**
     * @ingroup gui
     * @brief A QWidget customized for hal.
     *
     * The Widget class extends the QWidget with the following Q_SIGNALS: <ul>
     * <li> If the parent of a widget is changed a signal 'parentSetTo' is emitted.
     * <li >If the widget becomes hidden/shown the signals 'hidden'/'shown' are emitted.
     */
    class Widget : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        explicit Widget(QWidget* parent = nullptr);

        /**
         * Sets the parent of the widget to the specified widget. Overrides QWidget::setParent to emit the signal
         * Widget::parentSetTo after changing the parent.
         *
         * @param parent - The new parent
         */
        void setParent(QWidget* parent);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted whenever the parent of the widget changes.
         *
         * @param parent - The new parent
         */
        void parentSetTo(QWidget* parent);

        /**
         * Q_SIGNAL that is emitted after the widget was hidden.
         */
        void hidden();

        /**
         * Q_SIGNAL that is emitted after the widget was shown.
         */
        void shown();

    public Q_SLOTS:
        /**
         * Q_SLOT to hide the Widget. Overrides QWidget::hide to emit the signal Widget::hidden after hiding the Widget.
         */
        void hide();

        /**
         * Q_SLOT to show the Widget and its children. Overrides QWidget::show to emit the signal Widget::shown
         * after hiding the Widget.
         */
        void show();
    };
}
