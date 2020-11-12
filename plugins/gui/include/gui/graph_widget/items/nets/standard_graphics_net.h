//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

    class StandardGraphicsNet : public GraphicsNet
    {
    public:
        struct HLine
        {
            qreal mSmallX;
            qreal mBigX;
            qreal y;
        };

        struct VLine
        {
            qreal x;
            qreal mSmallY;
            qreal mBigY;
        };

        struct Lines
        {
            void appendHLine(const qreal mSmallX, const qreal mBigX, const qreal y);
            void appendVLine(const qreal x, const qreal mSmallY, const qreal mBigY);

            void mergeLines();

            int nLines() { return mHLines.size() + mVLines.size(); }
        private:
            QVector<HLine> mHLines;
            QVector<VLine> mVLines;

            friend StandardGraphicsNet;
        };

        static void loadSettings();
        static void updateAlpha();

        StandardGraphicsNet(Net* n, const Lines& l);

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
