#pragma once

#include <QGraphicsItem>

namespace hal {

    class WaveTimescale : public QGraphicsItem
    {
        int mMinorTicDistance;
        int mMaxTime;
        float mXmag;
    public:
        WaveTimescale(int max);
        void xScaleChanged(float mag);
        void setMaxTime(float tmax);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        QRectF boundingRect() const override;
    };
}
