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

#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include <QPixmap>

namespace hal
{
    class StandardGraphicsModule final : public GraphicsModule
    {
    public:
        static void loadSettings();
        static void updateAlpha();

        explicit StandardGraphicsModule(Module* m, bool adjust_size_to_grid = true);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        QPointF getInputScenePosition(const u32 mNetId, const QString& pin_type) const override;
        QPointF getOutputScenePosition(const u32 mNetId, const QString& pin_type) const override;
        QPointF endpointPositionByIndex(int index, bool isInput) const override;
        float   yEndpointDistance() const override;
        float   yTopPinDistance() const override;

    private:
        static qreal sAlpha;

        static QPen sPen;

        static QColor sTextColor;

        static QFont sTextFont[3];
        static QFont sPinFont;

        static qreal sTextFontHeight[3];

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

        void format(const bool& adjust_size_to_grid);

        QPointF mTextPosition[3];

        QVector<QPointF> mOutputPinPositions;
        static const int sIconPadding;
        static const QSize sIconSize;

        static QPixmap* sIconInstance;
        static const QPixmap& iconPixmap();
    };
}    // namespace hal
