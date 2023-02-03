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

#include "gui/overlay/overlay.h"

class QVBoxLayout;

namespace hal
{
    /**
     * @ingroup gui
     * @brief Container for a QWidget that overlays another one.
     *
     * The overlaying widget can be configures via setWidget. If the WidgetOverly is set to shown (WidgetOverly::show),
     * the configured QWidget will overlay the parent widget.
     */
    class WidgetOverlay : public Overlay
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The QWidget to overlay
         */
        WidgetOverlay(QWidget* parent = nullptr);

        /**
         * Configures the widget that should overlay the WidgetOverlay's parent.
         *
         * @param widget - The overlay widget
         */
        void setWidget(QWidget* widget);

        /**
         * Reset widget reference to nullptr. Does not delete widget since it has not the ownership.
         */
        void clearWidget() { mWidget = nullptr; }

    private:
        QVBoxLayout* mLayout;
        QWidget* mWidget;
    };
}
