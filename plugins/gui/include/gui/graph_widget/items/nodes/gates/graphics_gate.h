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

#include "gui/graph_widget/items/nodes/graphics_node.h"

#include <QPainterPath>

namespace hal
{
    class Gate;

    class Grouping;

    /**
     * @ingroup graph-visuals
     * @brief Abstract base class for gates.
     *
     * Abstract base class for GraphicsGates. Besides the functionality already provided by its superclass GraphicsNode
     * it stores the list of input and output pin type names of the GraphicsGate as well as the typename of the
     * GraphicsGate itself.
     */
    class GraphicsGate : public GraphicsNode
    {
    public:
        /**
         * Constructor.
         *
         * @param g - The underlying gate of the GraphicsGate
         */
        GraphicsGate(Gate* g);

        virtual QList<QLineF> connectors(u32 netId, bool isInput) const;

    protected:

        struct GraphicsGatePin
        {
            QString name;
            float textWidth;
            u32 netId;
            int index;
            bool isInput;
            float x;
            float y;
            QList<QLineF> connectors;
        };

        enum ShapeType { StandardShape, InverterShape, AndShape, OrShape };

        /**
         * Pointer to gate in hal core
         */
        Gate* mGate;

        /**
         * The gate type of the underlying gate
         */
        QString mType;

        ShapeType mShapeType;

        QPainterPath mPath;

        int mPathWidth;

        float mMaxInputPinWidth;

        float mMaxOutputPinWidth;

        /**
         * The input pin type identifiers of the underlying gate (from top to bottom)
         */
        QVector<struct GraphicsGatePin> mInputPinStruct;

        /**
         * The output pin type identifiers of the underlying gate (from top to bottom)
         */
        QVector<struct GraphicsGatePin> mOutputPinStruct;
    };
}
