#pragma once

#include <QGraphicsItem>

namespace hal {

    class WaveTimescale : public QGraphicsItem
    {
        int mMinorTicDistance;
        int mMaxValue;
        float mXmag;
    public:
        WaveTimescale(int max);
        void xScaleChanged(float mag);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        QRectF boundingRect() const override;
    };
}
