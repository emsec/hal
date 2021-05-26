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

#include "gui/graph_widget/items/graphics_item.h"

#include <memory>

namespace hal
{
    class Net;

    /**
     * @ingroup graph-visuals-net
     * @brief The basic net class all other nets inherit from.
     *
     * The GraphicsItem that represents a single net in the scene.
     */
    class GraphicsNet : public GraphicsItem
    {
    public:
        /**
         * Struct to store visuals properties of a graphics net, i.e. whether the net is visible or not as well as
         * the different colors and the brush style used to draw the net.
         */
        struct Visuals
        {
            bool mVisible;
            QColor color;
            Qt::PenStyle mPenStyle;
            bool mFillIcon;
            QColor mFillColor;
            Qt::BrushStyle mBrushStyle;
        };

        /**
         * Loads the cosmetic setting that will be applied to all GraphicsNets.
         */
        static void loadSettings();

        /**
         * Constructor.
         *
         * @param n - The underlying net
         */
        GraphicsNet(Net* n);

        /**
         * Get the rectangle that frames the entire net.
         *
         * @returns the bounding rectangle
         */
        QRectF boundingRect() const override;

        /**
         * Returns a painter path to draw this GraphicsNet.
         *
         * @returns the QPainterPath of the GraphicsNet
         */
        QPainterPath shape() const override;

        /**
         * Configures the passed visuals.
         *
         * @param v - The visuals to apply
         */
        virtual void setVisuals(const Visuals& v);

    protected:
        /** The line width of the GraphicsNet */
        static qreal sLineWidth;
        static qreal sShapeWidth;

        static QPen sPen;
        static QBrush sBrush;

        QRectF mRect;
        QPainterPath mShape;

        Qt::PenStyle mPenStyle;

        bool mFillIcon;
        QColor mFillColor;
        Qt::BrushStyle mBrushStyle;
    };
}
