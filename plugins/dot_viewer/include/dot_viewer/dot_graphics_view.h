// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS").
// All Rights reserved.
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

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>

namespace hal
{
    /**
     * The graphics view that displays a rendered DOT graph and handles zooming and panning.
     */
    class DotGraphicsView : public QGraphicsView
    {
        Q_OBJECT
    public:
        /**
         * Construct a new graphics view.
         *
         * @param[in] parent - The parent widget. Defaults to a `nullptr`.
         */
        DotGraphicsView(QWidget *parent = nullptr);

    public Q_SLOTS:
        /**
         * Zoom into the graph by one step, triggered by the zoom-in shortcut.
         */
        void handleZoomInShortcut();

        /**
         * Zoom out of the graph by one step, triggered by the zoom-out shortcut.
         */
        void handleZoomOutShortcut();

    protected:
        /**
         * Zoom the graph in or out when the mouse wheel is turned while the pan modifier is not held.
         *
         * @param[in] event - The wheel event.
         */
        void wheelEvent(QWheelEvent* event) override;

        /**
         * Remember the cursor position so that a subsequent drag can pan the graph.
         *
         * @param[in] event - The mouse event.
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * Pan the graph while the mouse is dragged with the pan modifier held.
         *
         * @param[in] event - The mouse event.
         */
        void mouseMoveEvent(QMouseEvent* event) override;

    private:
        /** The cursor position at which the current panning operation started. */
        QPoint mMovePosition;

        /** The keyboard modifier that has to be held to pan the graph instead of zooming it. */
        Qt::KeyboardModifier mPanModifier;

        /**
         * Scale the view by the given factor, unless that would exceed the minimum or maximum zoom level.
         *
         * @param[in] scaleFactor - The factor to scale the view by.
         */
        void scaleWithinLimits(qreal scaleFactor);
    };
}
