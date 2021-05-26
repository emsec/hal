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

#include <QGraphicsObject>

namespace hal
{
    /**
     * @ingroup graph-visuals
     * @brief An item that is drawn when a node is dragged through the scene.
     */
    class NodeDragShadow : public QGraphicsObject
    {
        Q_OBJECT

    public:
        enum class DragCue
        {
            Movable   = 0,
            Swappable = 1,
            Rejected  = 2
        };

        NodeDragShadow();

        void start(const QPointF& posF, const QSizeF& sizeF);
        void stop();

        qreal width() const;
        qreal height() const;
        QSizeF size() const;

        void setWidth(const qreal width);
        void setHeight(const qreal height);

        void setVisualCue(const DragCue cue);

        static void setLod(const qreal lod);
        static void loadSettings();

    protected:
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
        QRectF boundingRect() const override;
        QPainterPath shape() const override;

    private:
        static qreal sLod;
        static QPen sPen;
        static QColor sColorPen[];
        static QColor sColorSolid[];
        static QColor sColorTranslucent[];

        DragCue mCue;

        qreal mWidth;
        qreal mHeight;
    };
}    // namespace hal
