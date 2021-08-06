#pragma once

#include <QGraphicsLineItem>

namespace hal {

    class WaveScene;

    class WaveCursor : public QGraphicsLineItem
    {
        float mXmag;
    public:
        WaveCursor();
        void xScaleChanged(float mag);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

        QRectF boundingRect() const override;
        QPainterPath shape() const override;
    };

}
