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

#include "clock_tree_viewer/clock_tree_scene.h"

#include <QBrush>
#include <QGraphicsLineItem>
#include <QPainter>
#include <QPen>
#include <QRectF>
#include <algorithm>
#include <cmath>
#include <string>

namespace hal
{
    namespace
    {
        // For colors that were not given, there is nothing to take after: a drawing is always visible, if not pretty.
        const QColor FALLBACK_COLOR = Qt::black;

        QColor edge_color_of( const ClockTreeColors &colors )
        {
            return colors.edge.isValid() ? colors.edge : FALLBACK_COLOR;
        }

        QColor highlight_color_of( const ClockTreeColors &colors )
        {
            return colors.highlight.isValid() ? colors.highlight : edge_color_of( colors );
        }

        // The rings around highlighted gates are this large on the screen, however far the view is zoomed.
        constexpr double RING_RADIUS_PIXELS = 12.0;
        constexpr double RING_WIDTH_PIXELS = 2.0;

        // Above the lines and dots, which are at 0.
        constexpr double RING_Z_VALUE = 2.0;
    }  // namespace

    ClockTreeScene::ClockTreeScene( QObject *parent )
        : QGraphicsScene( parent )
        , mStyle{ 0.0, 0.0, 0.0, 0.0 }
        , mHasLayout( false )
    {
    }

    QPointF ClockTreeScene::fromChip( double x, double y )
    {
        return QPointF( x, -y );
    }

    QPointF ClockTreeScene::toChip( const QPointF &scenePosition )
    {
        return QPointF( scenePosition.x(), -scenePosition.y() );
    }

    QColor ClockTreeScene::colorOfNode( const ClockTreeColors &colors, int index ) const
    {
        const QColor color = colors.nodes.value( index );
        return color.isValid() ? color : edge_color_of( colors );
    }

    void ClockTreeScene::setLayout( const ctv::Layout &layout, const ctv::Style &style, const ClockTreeColors &colors )
    {
        clearLayout();

        mStyle = style;
        mEdgeColor = edge_color_of( colors );
        mHighlightColor = highlight_color_of( colors );

        const QPen edge_pen( mEdgeColor, style.edge_width );
        mEdgeItems.reserve( static_cast<int>( layout.edges.size() ) );
        for( const ctv::Edge &edge : layout.edges )
        {
            const ctv::Node &from = layout.nodes[edge.from];
            const ctv::Node &to = layout.nodes[edge.to];

            mEdgeItems.push_back( addLine( QLineF( fromChip( from.x, from.y ), fromChip( to.x, to.y ) ), edge_pen ) );
        }

        // The nodes come last, so they are drawn on top of the lines.
        mNodeItems.reserve( static_cast<int>( layout.nodes.size() ) );
        mNodeColors.reserve( static_cast<int>( layout.nodes.size() ) );
        mNodeCenters.reserve( static_cast<int>( layout.nodes.size() ) );
        for( std::size_t idx = 0; idx < layout.nodes.size(); idx++ )
        {
            const ctv::Node &node = layout.nodes[idx];
            const QPointF center = fromChip( node.x, node.y );
            const QColor color = colorOfNode( colors, static_cast<int>( idx ) );

            mNodeItems.push_back( addEllipse( QRectF( center.x() - style.node_radius,
                                                      center.y() - style.node_radius,
                                                      2.0 * style.node_radius,
                                                      2.0 * style.node_radius ),
                                              QPen( color, style.edge_width ),
                                              QBrush( color ) ) );
            mNodeColors.push_back( color );
            mNodeCenters.push_back( center );
        }

        // Taken now, while there is nothing but lines and dots: the rings around highlighted gates are as large as they are
        // on the screen, so what they take up in the scene depends on the view.
        mContentRect = itemsBoundingRect();

        const double margin = style.picture_margin;
        setSceneRect( mContentRect.adjusted( -margin, -margin, margin, margin ) );

        mHasLayout = true;
    }

    void ClockTreeScene::setColors( const ClockTreeColors &colors )
    {
        if( !mHasLayout )
        {
            return;
        }

        const QColor edge = edge_color_of( colors );
        if( edge != mEdgeColor )
        {
            mEdgeColor = edge;

            const QPen edge_pen( edge, mStyle.edge_width );
            for( QGraphicsLineItem *item : mEdgeItems )
            {
                item->setPen( edge_pen );
            }
        }

        // A dot that is not given a color follows the lines, so this has to look at every one of them.
        for( int idx = 0; idx < mNodeItems.size(); idx++ )
        {
            const QColor color = colorOfNode( colors, idx );
            if( color == mNodeColors[idx] )
            {
                continue;
            }

            mNodeColors[idx] = color;
            mNodeItems[idx]->setPen( QPen( color, mStyle.edge_width ) );
            mNodeItems[idx]->setBrush( QBrush( color ) );
        }

        const QColor highlight = highlight_color_of( colors );
        if( highlight != mHighlightColor )
        {
            mHighlightColor = highlight;

            QPen ring_pen( highlight, RING_WIDTH_PIXELS );
            ring_pen.setCosmetic( true );
            for( QGraphicsEllipseItem *ring : mRingItems )
            {
                ring->setPen( ring_pen );
            }
        }
    }

    void ClockTreeScene::setHighlight( const QVector<int> &nodes )
    {
        if( !mHasLayout )
        {
            return;
        }

        QVector<int> wanted;
        wanted.reserve( nodes.size() );
        for( const int node : nodes )
        {
            if( node >= 0 && node < mNodeCenters.size() )
            {
                wanted.push_back( node );
            }
        }
        std::sort( wanted.begin(), wanted.end() );
        wanted.erase( std::unique( wanted.begin(), wanted.end() ), wanted.end() );

        if( wanted == mHighlighted )
        {
            return;
        }

        // Deleting an item takes it out of the scene.
        qDeleteAll( mRingItems );
        mRingItems.clear();

        QPen ring_pen( mHighlightColor, RING_WIDTH_PIXELS );
        ring_pen.setCosmetic( true );

        mRingItems.reserve( wanted.size() );
        for( const int node : wanted )
        {
            QGraphicsEllipseItem *ring = new QGraphicsEllipseItem(
                -RING_RADIUS_PIXELS, -RING_RADIUS_PIXELS, 2.0 * RING_RADIUS_PIXELS, 2.0 * RING_RADIUS_PIXELS );
            ring->setFlag( QGraphicsItem::ItemIgnoresTransformations );
            ring->setPos( mNodeCenters[node] );
            ring->setPen( ring_pen );
            ring->setBrush( Qt::NoBrush );
            ring->setZValue( RING_Z_VALUE );

            addItem( ring );
            mRingItems.push_back( ring );
        }

        mHighlighted = wanted;
    }

    QRectF ClockTreeScene::highlightedRect() const
    {
        if( mHighlighted.isEmpty() )
        {
            return QRectF();
        }

        // Not QRectF::united, which leaves out a rectangle without an area, such as a single point.
        double left = mNodeCenters[mHighlighted.first()].x();
        double right = left;
        double top = mNodeCenters[mHighlighted.first()].y();
        double bottom = top;

        for( const int node : mHighlighted )
        {
            left = std::min( left, mNodeCenters[node].x() );
            right = std::max( right, mNodeCenters[node].x() );
            top = std::min( top, mNodeCenters[node].y() );
            bottom = std::max( bottom, mNodeCenters[node].y() );
        }

        return QRectF( QPointF( left, top ), QPointF( right, bottom ) );
    }

    void ClockTreeScene::clearLayout()
    {
        // The items are deleted by the scene, only the pointers to them are ours.
        mEdgeItems.clear();
        mNodeItems.clear();
        mNodeColors.clear();
        mNodeCenters.clear();
        mRingItems.clear();
        mHighlighted.clear();
        mContentRect = QRectF();

        clear();

        // Without a rectangle of its own the scene is as large as its items again.
        setSceneRect( QRectF() );

        mHasLayout = false;
    }

    Result<QImage> ClockTreeScene::renderPicture( const QColor &background )
    {
        if( !mHasLayout )
        {
            return ERR( "there is no clock tree to take a picture of" );
        }

        const double margin = mStyle.picture_margin;
        const QRectF source = mContentRect.adjusted( -margin, -margin, margin, margin );

        const double width = std::ceil( source.width() * mStyle.pixels_per_unit );
        const double height = std::ceil( source.height() * mStyle.pixels_per_unit );

        if( width > ctv::MAX_PICTURE_SIDE_PIXELS || height > ctv::MAX_PICTURE_SIDE_PIXELS )
        {
            return ERR( "the picture would be " + std::to_string( static_cast<long long>( width ) ) + " x "
                        + std::to_string( static_cast<long long>( height ) ) + " pixels, more than the limit of "
                        + std::to_string( static_cast<long long>( ctv::MAX_PICTURE_SIDE_PIXELS ) )
                        + " pixels per side, use a smaller pixels_per_unit" );
        }

        QImage image( static_cast<int>( width ), static_cast<int>( height ), QImage::Format_RGB32 );
        if( image.isNull() )
        {
            return ERR( "could not allocate a picture of " + std::to_string( static_cast<long long>( width ) ) + " x "
                        + std::to_string( static_cast<long long>( height ) ) + " pixels" );
        }

        image.fill( background.isValid() ? background : QColor( Qt::white ) );

        QPainter painter( &image );
        painter.setRenderHint( QPainter::Antialiasing );
        render( &painter, QRectF( 0, 0, width, height ), source );
        painter.end();

        return OK( image );
    }
}  // namespace hal
