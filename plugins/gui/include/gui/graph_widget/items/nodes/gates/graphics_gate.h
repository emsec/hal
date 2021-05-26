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

#include "gui/graph_widget/items/nodes/graphics_node.h"

#include <memory>
#include <QHash>

namespace hal
{
    class Gate;

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

    protected:
        /**
         * The gate type of the underlying gate
         */
        QString mType;

        /**
         * The input pin type identifiers of the underlying gate (from top to bottom)
         */
        QVector<QString> mInputPins;

        /**
         * The output pin type identifiers of the underlying gate (from top to bottom)
         */
        QVector<QString> mOutputPins;
    };
}
