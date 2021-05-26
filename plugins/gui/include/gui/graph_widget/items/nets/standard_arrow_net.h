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

#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"

namespace hal {

    /**
     * @ingroup graph-visuals-net
     * @brief A standard net that has parts of a separated net.
     *
     * The GraphicsNet that can be used to display nets in the scene.
     * Currently only used in GraphLayouter::alternateLayout().
     */
    class StandardArrowNet : public StandardGraphicsNet
    {
        ArrowSeparatedNet mArrowNet;
    public:
        /**
         * Constructor.
         *
         * @param n - The underlying net
         * @param l - The lines this GraphicsNet consists of
         */
        StandardArrowNet(Net* n, const Lines& l);
        /**
         * Adds an input position.
         *
         * @param pos - The position.
         */
        void setInputPosition(const QPointF &pos);

        /**
         * Adds an output position.
         *
         * @param pos - The position.
         */
        void setOutputPosition(const QPointF &pos);

        /**
         * Sets the given visuals.
         *
         * @param v - The visuals to set.
         */
        virtual void setVisuals(const Visuals& v);

        /**
         * Overwritten qt function to draw the net.
         *
         * @param painter - The painter to draw.
         * @param option - The options.
         * @param widget - The widget to draw on.
         */
        void paint(QPainter* painter,
                   const QStyleOptionGraphicsItem* option,
                   QWidget* widget) override;
    };

}
