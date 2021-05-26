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

#include "gui/graph_widget/items/nets/separated_graphics_net.h"

namespace hal
{
    /**
     * @ingroup graph-visuals-net
     * @brief Nets whose output and input can be labeled (e.g. from GND/VCC)
     *
     * The SeparatedGraphicsNet that is used to visualize constant inputs (and outputs from GND/VCC gates).
     * The inputs and outputs of the net are displayed as a small box that contains the constant's value as a text.
     */
    class LabeledSeparatedNet : public SeparatedGraphicsNet
    {
    public:
        /**
         * Loads the cosmetic setting that will be applied to all LabeledSeparatedNet.
         */
        static void loadSettings();

        /**
         * Constructor.
         *
         * @param n - The underlying net
         * @param text - The text that is displayed in the in- and output boxes.
         */
        LabeledSeparatedNet(Net* n, const QString& text);

        /**
         * Draws the LabeledSeparatedNet in the scene.
         *
         * @param painter - The used QPainter
         * @param option - The styling options. Used to access the state (option->state).
         * @param widget - The widget to paint on
         */
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

        /**
         * Add the an input pin as a destination of the LabeledSeparatedNet
         *
         * @param scene_position - The scene position of the input pin
         */
        void addInput(const QPointF& scene_position) override;

        /**
         * Add the an output pin as a source of the ALabeledSeparatedNet
         *
         * @param scene_position - The scene position of the output pin
         */
        void addOutput(const QPointF& scene_position) override;

        /**
         * Get the width of one entire input (including the line and the box) of the LabeledSeparatedNet
         *
         * @returns the width of an input
         */
        qreal inputWidth() const override;

        /**
         * Get the width of one entire output (including the line and the box) of the LabeledSeparatedNet
         *
         * @returns the width of an output
         */
        qreal outputWidth() const override;

    private:
        static qreal sWireLength;
        static qreal sTextOffset;

        static QFont sFont;
        static qreal sFontHeight;
        static qreal sFontAscend;

        QString mText;
        qreal mTextWidth;
    };
}
