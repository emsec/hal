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

namespace hal
{
    /**
     * @ingroup graph-visuals-net
     * @brief Abstract base class for separated nets (e.g. ArrowSeparatedNet)
     *
     * Abstract base class used for all types of separated GraphicsNets. <br>
     * An example for SeparatedGraphicNets are the little arrow shaped boxes that represent global in- and outputs. They
     * also appear in labeled versions to represent constant signals from GND/VCC gates.
     */
    class SeparatedGraphicsNet : public GraphicsNet
    {
    public:
        /**
         * Updates the alpha value (transparency) of the SeparatedGraphicsNet based on the current level-of-detail.
         */
        static void updateAlpha();

        /**
         * Constructor.
         *
         * @param n - The underlying net
         */
        SeparatedGraphicsNet(Net* n);

        /**
         * Add the an input pin as a destination of the SeparatedGraphicsNet
         *
         * @param scene_position - The scene position of the input pin
         */
        virtual void addInput(const QPointF& scene_position) = 0;

        /**
         * Add the an output pin as a source of the SeparatedGraphicsNet
         *
         * @param scene_position - The scene position of the output pin
         */
        virtual void addOutput(const QPointF& scene_position) = 0;

        /**
         * Get the width of one entire input (including the line and the box) of the SeparatedGraphicsNet
         *
         * @returns the width of an input
         */
        virtual qreal inputWidth() const = 0;

        /**
         * Get the width of one entire output (including the line and the box) of the SeparatedGraphicsNet
         *
         * @returns the width of an output
         */
        virtual qreal outputWidth() const = 0;

        /**
         * Commits the path and sets its bounding rect.
         */
        virtual void finalize();

    protected:
        static qreal sAlpha;

        QVector<QPointF> mInputPositions;
        QVector<QPointF> mOutputPositions;
    };
}
