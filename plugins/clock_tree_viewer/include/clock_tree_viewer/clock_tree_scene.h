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

#include "clock_tree_viewer/clock_tree_layout.h"
#include "hal_core/utilities/result.h"

#include <QColor>
#include <QGraphicsScene>
#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QVector>

class QGraphicsEllipseItem;
class QGraphicsLineItem;

namespace hal
{
    /**
     * The colors of a drawing of a clock tree. Where they come from is up to the caller, the scene only paints them.
     */
    struct ClockTreeColors
    {
        /** The color of the lines. */
        QColor edge;

        /** The color of every dot, in the order of `ctv::Layout::nodes`. A dot without an entry gets the color of the lines. */
        QVector<QColor> nodes;

        /** The color of the rings around highlighted gates. Without one, they get the color of the lines. */
        QColor highlight;
    };

    /**
     * The scene that holds the drawing of a clock tree: a line for every connection and a dot for every gate.
     *
     * Gates can be highlighted by a ring around them. The ring has the same size on the screen at every zoom, so a single
     * gate can be found in a drawing of the whole chip, where the dot of a gate is far smaller than a pixel.
     *
     * The scene paints no background of its own, the view shows its own. Chip coordinates have their origin in the lower
     * left corner, the scene's y axis points down. The scene therefore stores the chip's y coordinate negated, use
     * `fromChip` and `toChip` to convert.
     */
    class ClockTreeScene : public QGraphicsScene
    {
      public:
        /**
         * Construct an empty scene.
         *
         * @param[in] parent - The parent object. Defaults to a `nullptr`.
         */
        explicit ClockTreeScene( QObject *parent = nullptr );

        /**
         * Get the position in the scene of a position on the chip.
         *
         * @param[in] x - The x coordinate on the chip.
         * @param[in] y - The y coordinate on the chip.
         * @returns The position in the scene.
         */
        static QPointF fromChip( double x, double y );

        /**
         * Get the position on the chip of a position in the scene.
         *
         * @param[in] scenePosition - The position in the scene.
         * @returns The position on the chip.
         */
        static QPointF toChip( const QPointF &scenePosition );

        /**
         * Replace the drawing. The scene is as large as the tree plus the picture margin on every side.
         *
         * @param[in] layout - The gates and connections to draw.
         * @param[in] style - The sizes to draw them with.
         * @param[in] colors - The colors to draw them with.
         */
        void setLayout( const ctv::Layout &layout, const ctv::Style &style, const ClockTreeColors &colors );

        /**
         * Paint the current drawing in other colors. Only what actually changes color is repainted.
         *
         * @param[in] colors - The new colors, for the gates and connections of the layout that is shown.
         */
        void setColors( const ClockTreeColors &colors );

        /**
         * Highlight gates by a ring around each of them, instead of the ones that were highlighted before.
         *
         * @param[in] nodes - The gates, as indices into `ctv::Layout::nodes`. Indices that are out of range are ignored.
         */
        void setHighlight( const QVector<int> &nodes );

        /**
         * Remove the drawing.
         */
        void clearLayout();

        /**
         * Check whether a drawing is shown.
         *
         * @returns `true` if there is a drawing, `false` otherwise.
         */
        [[nodiscard]] bool hasLayout() const
        {
            return mHasLayout;
        }

        /**
         * Get the sizes the current drawing was made with.
         *
         * @returns The sizes, meaningless without a drawing.
         */
        [[nodiscard]] const ctv::Style &style() const
        {
            return mStyle;
        }

        /**
         * Get the color the lines are painted in.
         *
         * @returns The color, meaningless without a drawing.
         */
        [[nodiscard]] QColor edgeColor() const
        {
            return mEdgeColor;
        }

        /**
         * Get the color a dot is painted in.
         *
         * @param[in] index - The index of the gate in `ctv::Layout::nodes`.
         * @returns The color, an invalid color if there is no such gate.
         */
        [[nodiscard]] QColor nodeColor( int index ) const
        {
            return mNodeColors.value( index );
        }

        /**
         * Get the gates that are highlighted.
         *
         * @returns The indices into `ctv::Layout::nodes`, sorted.
         */
        [[nodiscard]] const QVector<int> &highlightedNodes() const
        {
            return mHighlighted;
        }

        /**
         * Get the area that the highlighted gates are in.
         *
         * @returns The smallest rectangle in the scene that holds the centers of all of them. It has no size if there is only
         *          one gate, and it is a null rectangle if there is none: check `highlightedNodes` to tell them apart.
         */
        QRectF highlightedRect() const;

        /**
         * Render the whole drawing and its margin into a picture, independent of any view and its zoom. Highlights are part
         * of the picture.
         *
         * @param[in] background - The color of the background, which the scene does not paint.
         * @returns The picture on success, an error if there is no drawing or the picture would be too large.
         */
        Result<QImage> renderPicture( const QColor &background );

      private:
        /**
         * Get the color a dot gets: the one it is given, or that of the lines.
         */
        QColor colorOfNode( const ClockTreeColors &colors, int index ) const;

        ctv::Style mStyle;

        bool mHasLayout;

        QColor mEdgeColor;

        QVector<QGraphicsLineItem *> mEdgeItems;

        QVector<QGraphicsEllipseItem *> mNodeItems;

        QVector<QColor> mNodeColors;

        QVector<QPointF> mNodeCenters;

        /** The area of the lines and dots, which the rings around highlighted gates do not enlarge. */
        QRectF mContentRect;

        QColor mHighlightColor;

        QVector<QGraphicsEllipseItem *> mRingItems;

        QVector<int> mHighlighted;
    };
}  // namespace hal
