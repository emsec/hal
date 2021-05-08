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

#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

namespace hal
{
    /**
     * @ingroup graph-visuals-gate
     * @brief The standard gate to visualize netlist gates.
     *
     * The GraphicsGate that is (currently) used to display gates in the scene.
     */
    class StandardGraphicsGate final : public GraphicsGate
    {
    public:
        /**
         * Loads the cosmetic setting that will be applied to all GraphicsGates.
         */
        static void loadSettings();

        /**
         * Updates the alpha value (transparency) of the GraphicsGate based on the current level-of-detail.
         */
        static void updateAlpha();

        /**
         * Constructor.
         *
         * @param g - The underlying gate of the StandardGraphicsGate
         * @param adjust_size_to_grid - <b>true</b> if the size of the GraphicsGate should be extended so that its
         *                              height and width are a multiple of the grid size.
         */
        StandardGraphicsGate(Gate* g, const bool adjust_size_to_grid = true);

        /**
         * Draws the StandardGraphicsGate in the scene.
         *
         * @param painter - The used QPainter
         * @param option - The styling options. Used to access the state (option->state).
         * @param widget - The widget to paint on
         */
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        /**
         * Given the id of a net and the type of the pin of one of its destinations, this function returns
         * the position of the input pin in the scene.
         *
         * @param mNetId - The net id
         * @param pin_type - The input pin type
         * @returns The pin's position in the scene
         */
        QPointF getInputScenePosition(const u32 mNetId, const QString& pin_type) const override;

        /**
         * Given the id of a net and the type of the pin of one of its sources, this function returns
         * the position of the output pin in the scene.
         *
         * @param mNetId - The net id
         * @param pin_type - The output pin type
         * @returns The pin's position in the scene
         */
        QPointF getOutputScenePosition(const u32 mNetId, const QString& pin_type) const override;

        /**
         * Given the index of an input/output pin, this function returns the position of this pin in the scene. <br>
         * (Pins are positioned from top (idx=0) to bottom (idx=maxIdx))
         *
         * @param index - The index of the pin
         * @param isInput - <b>true</b> if the pin is an input pin <br>
         *                - <b>false</b> if the pin is an output pin
         * @returns the position of the endpoint in the scene
         */
        QPointF endpointPositionByIndex(int index, bool isInput) const override;

        /**
         * Gets the vertical space between two endpoints (in scene coordinates).
         *
         * @returns the vertical space between two endpoints
         */
        float   yEndpointDistance() const override;

        /**
         * Gets the distance between the top edge of the node and the first pin (in scene coordinates).
         *
         * @returns the vertical space between the top edge of the node and the first pin
         */
        float   yTopPinDistance() const override;

    private:
        static qreal sAlpha;

        static QPen sPen;

        static QColor sTextColor;

        static QFont sTextFont[2];
        static QFont sPinFont;

        static qreal sTextFontHeight[2];

        static qreal sColorBarHeight;

        static qreal sPinInnerHorizontalSpacing;
        static qreal sPinOuterHorizontalSpacing;

        static qreal sPinInnerVerticalSpacing;
        static qreal sPinOuterVerticalSpacing;
        static qreal sPinUpperVerticalSpacing;
        static qreal sPinLowerVerticalSpacing;

        static qreal sPinFontHeight;
        static qreal sPinFontAscent;
        static qreal sPinFontDescent;
        static qreal sPinFontBaseline;

        static qreal sInnerNameTypeSpacing;
        static qreal sOuterNameTypeSpacing;

        static qreal sFirstPinY;
        static qreal sPinYStride;

        void format(const bool& adjust_size_to_grid);

        QPointF mTextPosition[2];

        QVector<QPointF> mOutputPinPositions;
        static const int sIconPadding;
        static const QSize sIconSize;

        static QPixmap* sIconInstance;
        static const QPixmap& iconPixmap();
    };
}
