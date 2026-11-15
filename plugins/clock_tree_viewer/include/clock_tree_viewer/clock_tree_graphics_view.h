// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
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

class QKeyEvent;
class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QWheelEvent;

namespace hal
{
    class ClockTreeScene;

    /**
     * The view that displays a clock tree. The mouse wheel and the keys '+' and '-' zoom, dragging pans. The zoom stays
     * within limits that follow from the drawing, so neither does the tree vanish nor can a gate be magnified beyond
     * use.
     */
    class ClockTreeGraphicsView : public QGraphicsView
    {
        Q_OBJECT

      public:
        /**
         * Construct a new view.
         *
         * @param[in] scene - The scene to display, which has to outlive the view.
         * @param[in] parent - The parent widget. Defaults to a `nullptr`.
         */
        explicit ClockTreeGraphicsView( ClockTreeScene *scene, QWidget *parent = nullptr );

        /**
         * Zoom so that the whole scene is visible. If the view is not shown yet and thus has no size to fit to, this
         * happens as soon as it is.
         */
        void fitToScene();

        /**
         * Zoom so that an area of the scene is visible, with some air around it. The zoom stays within its limits, so a
         * single gate is shown at a size that can be looked at, not as large as the view.
         *
         * @param[in] area - The area in the scene. It may be a single point.
         */
        void showArea( const QRectF &area );

      Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when the mouse moves over the view.
         *
         * @param[in] x - The x coordinate on the chip below the mouse.
         * @param[in] y - The y coordinate on the chip below the mouse.
         */
        void coordinatesChanged( double x, double y );

      protected:
        void wheelEvent( QWheelEvent *event ) override;
        void mouseMoveEvent( QMouseEvent *event ) override;
        void keyPressEvent( QKeyEvent *event ) override;
        void showEvent( QShowEvent *event ) override;
        void resizeEvent( QResizeEvent *event ) override;

      private:
        /**
         * Zoom so that a rectangle of the scene is visible, whatever the zoom limits are.
         */
        void fitRect( const QRectF &rect );

        /**
         * Scale the view by the given factor, unless that would leave the zoom limits.
         */
        void zoomBy( double factor );

        /**
         * Get the scale at which the whole scene is visible.
         */
        [[nodiscard]] double fitScale() const;

        ClockTreeScene *mScene;

        bool mFitPending;
    };
}  // namespace hal
