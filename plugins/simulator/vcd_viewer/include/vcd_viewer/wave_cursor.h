#pragma once

#include <QGraphicsItem>

namespace hal {

    class WaveScene;

    class WaveCursorLabel : public QGraphicsItem
    {
        QRectF mBoundingRect;
    public:
        WaveCursorLabel(const QRectF& bbox, QGraphicsItem* parentItem);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
        QRectF boundingRect() const override;
    };

    class WaveCursor : public QGraphicsItem
    {
        float mXmag;
    public:
        WaveCursor();
        void xScaleChanged(float mag);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
        QRectF boundingRect() const override;
        QPainterPath shape() const override;
    };

}
