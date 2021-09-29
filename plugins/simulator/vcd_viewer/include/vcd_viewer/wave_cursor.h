#pragma once

#include <QGraphicsItem>

namespace hal {

    class WaveScene;

    class WaveCursor : public QGraphicsItem
    {
        float mXmag;
    public:
        WaveCursor();
        void xScaleChanged(float mag);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        QRectF boundingRect() const override;
        QPainterPath shape() const override;
    };

}
