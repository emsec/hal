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

#include <QFrame>

namespace hal
{
    /**
     * @ingroup gui
     * @brief The Overlay overlays its parent.
     *
     * The Overlay's parent will be overlayed by this QFrame if it is shown (Overlay:show). To hide the Overlay use
     * Overlay::hide. This class is used as an abstract class for diverse overlay contents.
     */
    class Overlay : public QFrame
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The QWidget to overlay
         */
        explicit Overlay(QWidget* parent = nullptr);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted whenever a click withing the overlay was registered.
         */
        void clicked();

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;
        bool event(QEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;

    private:
        void handleParentChanged();
    };
}
