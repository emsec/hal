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

#include "gui/graph_widget/items/nets/graphics_net.h"

#include <QLineF>
#include <QVector>

namespace hal
{
    class Net;

    /**
     * @ingroup graph-visuals-net
     * @brief The most basic net to display the standard nets.
     *
     * The GraphicsNet that is (currently) used to display nets in the scene. It provides functions to build up a net
     * line by line (using the Lines struct).
     */
    class StandardGraphicsNet : public GraphicsNet
    {
    public:
        /**
         * Represents a horizontal line of a net.
         */
        struct HLine
        {
            /** The x-coordinate of the starting point of the line */
            qreal mSmallX;
            /** The x-coordinate of the end point of the line */
            qreal mBigX;
            /** The y-coordinate of the horizontal line */
            qreal y;
        };

        /**
         * Represents a vertical line of a net.
         */
        struct VLine
        {
            /** The x-coordinate of the vertical line */
            qreal x;
            /** The y-coordinate of the starting point of the line */
            qreal mSmallY;
            /** The y-coordinate of the end point of the line */
            qreal mBigY;
        };

        /**
         * Struct to store all lines of a StandardGraphicsNet.
         */
        struct Lines
        {
            /**
             * Adds a horizontal line.
             *
             * @param mSmallX - The x-coordinate of the starting point of the line (lower x-coord)
             * @param mBigX - The x-coordinate of the end point of the line (higher x-coord)
             * @param y - The y-coordinate of the horizontal line
             */
            void appendHLine(const qreal mSmallX, const qreal mBigX, const qreal y);

            /**
             * Adds a vertical line.
             *
             * @param x - The x-coordinate of the vertical line
             * @param mSmallY - The y-coordinate of the starting point of the line (lower y-coord)
             * @param mBigY - The y-coordinate of the end point of the line (higher y-coord)
             */
            void appendVLine(const qreal x, const qreal mSmallY, const qreal mBigY);

            /**
             * Merges all horizontal and vertical lines so that overlapping lines are replaced by only a single line.
             */
            void mergeLines();

            /**
             * Gets the total amount of lines (includes horizontal AND vertical lines).
             *
             * @returns the total amount of lines
             */
            int nLines() { return mHLines.size() + mVLines.size(); }
        private:
            /** All horizontal lines */
            QVector<HLine> mHLines;
            /** All vertical lines */
            QVector<VLine> mVLines;

            friend StandardGraphicsNet;
        };

        /**
         * Loads the cosmetic setting that will be applied to all StandardGraphicsNets.
         */
        static void loadSettings();

        /**
         * Updates the alpha value (transparency) of the StandardGraphicsNet based on the current level-of-detail.
         */
        static void updateAlpha();

        /**
         * Constructor.
         *
         * @param n - The undelying net of the StandardGraphicsNet
         * @param l - The lines this GraphicsNet consists of
         */
        StandardGraphicsNet(Net* n, const Lines& l);

        /**
         * Draws the StandardGraphicsNet in the scene.
         *
         * @param painter - The used QPainter
         * @param option - The styling options. Used to access the state (option->state).
         * @param widget - The widget to paint on
         */
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static qreal sAlpha;

        static qreal sWireLength;

        static qreal sLeftArrowOffset;
        static qreal sRightArrowOffset;

        static qreal sArrowLeftXShift;
        static qreal sArrowRightXShift;
        static qreal sArrowSideLength;

        static qreal sArrowWidth;
        static qreal sArrowHeight;

        static QPainterPath sArrow;

        static qreal sSplitRadius;

        QVector<QLineF> mLines;
        QVector<QPointF> mSplits;
    };
}
