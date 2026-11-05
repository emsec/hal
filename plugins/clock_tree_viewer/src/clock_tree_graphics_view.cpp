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

#include "clock_tree_viewer/clock_tree_graphics_view.h"

#include "clock_tree_viewer/clock_tree_scene.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <algorithm>
#include <cmath>

namespace hal
{
    namespace
    {
        // The factor the zoom changes by with one notch of the mouse wheel or one key press.
        constexpr double ZOOM_STEP = 1.25;

        // The tree may be zoomed out to half of the scale at which it fills the view ...
        constexpr double MIN_ZOOM_OF_FIT = 0.5;

        // ... and in until a single gate is this many pixels across.
        constexpr double MAX_NODE_DIAMETER_PIXELS = 400.0;

        // Around an area that is shown there is at least this many gate diameters of air.
        constexpr double AREA_AIR_NODE_DIAMETERS = 10.0;
    }  // namespace

    ClockTreeGraphicsView::ClockTreeGraphicsView( ClockTreeScene *scene, QWidget *parent )
        : QGraphicsView( scene, parent )
        , mScene( scene )
        , mFitPending( false )
    {
        setRenderHint( QPainter::Antialiasing );
        setDragMode( QGraphicsView::ScrollHandDrag );
        setViewportUpdateMode( QGraphicsView::BoundingRectViewportUpdate );
        setTransformationAnchor( QGraphicsView::AnchorUnderMouse );

        // Without it, the view only hears about the mouse while a button is down.
        setMouseTracking( true );
    }

    void ClockTreeGraphicsView::fitToScene()
    {
        if( !isVisible() || viewport()->width() < 2 || viewport()->height() < 2 )
        {
            mFitPending = true;
            return;
        }

        mFitPending = false;
        fitRect( sceneRect() );
    }

    void ClockTreeGraphicsView::fitRect( const QRectF &rect )
    {
        // fitInView measures the viewport with the scroll bars that happen to be showing, and they are gone once
        // everything fits, which leaves a gap around the tree. So fit without them.
        const Qt::ScrollBarPolicy horizontal = horizontalScrollBarPolicy();
        const Qt::ScrollBarPolicy vertical = verticalScrollBarPolicy();

        setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

        fitInView( rect, Qt::KeepAspectRatio );

        setHorizontalScrollBarPolicy( horizontal );
        setVerticalScrollBarPolicy( vertical );
    }

    void ClockTreeGraphicsView::showArea( const QRectF &area )
    {
        if( !mScene->hasLayout() )
        {
            return;
        }

        // Some air around the gates, so they are not at the edge of the view: a tenth of the area, but at least a few
        // gates wide, which is all there is around a single gate.
        const double air = std::max( 0.1 * std::max( area.width(), area.height() ),
                                     AREA_AIR_NODE_DIAMETERS * 2.0 * mScene->style().node_radius );

        fitRect( area.adjusted( -air, -air, air, air ) );

        // Fitting a few gates zooms in further than is of use, bring the zoom back within its limits.
        zoomBy( 1.0 );
        centerOn( area.center() );
    }

    double ClockTreeGraphicsView::fitScale() const
    {
        const QRectF rect = sceneRect();
        if( rect.isEmpty() )
        {
            return 1.0;
        }

        return std::min( viewport()->width() / rect.width(), viewport()->height() / rect.height() );
    }

    void ClockTreeGraphicsView::zoomBy( double factor )
    {
        if( !mScene->hasLayout() )
        {
            return;
        }

        const double current = transform().m11();
        const double fit = fitScale();

        const double minimum = MIN_ZOOM_OF_FIT * fit;
        const double maximum = std::max( 2.0 * fit, MAX_NODE_DIAMETER_PIXELS / ( 2.0 * mScene->style().node_radius ) );

        const double target = std::clamp( current * factor, minimum, maximum );
        if( target == current )
        {
            return;
        }

        scale( target / current, target / current );
    }

    void ClockTreeGraphicsView::wheelEvent( QWheelEvent *event )
    {
        // 120 is one notch of a mouse wheel. A trackpad reports many small steps, which zoom correspondingly little.
        const double notches = event->angleDelta().y() / 120.0;
        if( notches == 0.0 )
        {
            event->ignore();
            return;
        }

        zoomBy( std::pow( ZOOM_STEP, notches ) );
        event->accept();
    }

    void ClockTreeGraphicsView::mouseMoveEvent( QMouseEvent *event )
    {
        const QPointF chip = ClockTreeScene::toChip( mapToScene( event->position().toPoint() ) );
        Q_EMIT coordinatesChanged( chip.x(), chip.y() );

        QGraphicsView::mouseMoveEvent( event );
    }

    void ClockTreeGraphicsView::keyPressEvent( QKeyEvent *event )
    {
        if( event->key() == Qt::Key_Plus )
        {
            zoomBy( ZOOM_STEP );
        }
        else if( event->key() == Qt::Key_Minus )
        {
            zoomBy( 1.0 / ZOOM_STEP );
        }
        else
        {
            QGraphicsView::keyPressEvent( event );
        }
    }

    void ClockTreeGraphicsView::showEvent( QShowEvent *event )
    {
        QGraphicsView::showEvent( event );

        if( mFitPending )
        {
            fitToScene();
        }
    }

    void ClockTreeGraphicsView::resizeEvent( QResizeEvent *event )
    {
        QGraphicsView::resizeEvent( event );

        // The size a view has when it is first shown is not necessarily its final one.
        if( mFitPending )
        {
            fitToScene();
        }
    }
}  // namespace hal
