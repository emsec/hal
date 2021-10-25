#pragma once

#include <QGraphicsItem>
#include <QLine>

namespace hal {

    class WaveData;

    class WaveItem : public QGraphicsItem
    {
        const WaveData* mData;
        QVector<QLine> mSolidLines;
        QVector<QLineF> mDotLines;

        int mYoffset;
        float mMaxTime;

        void construct();
    public:
        WaveItem(const WaveData* dat, int off);
        ~WaveItem();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        void setYoffset(int val);
        void setWavedata(const WaveData* dat);
        void setMaxTime(float tmax);
        float maxTime() const { return mMaxTime; }
        const WaveData* wavedata() const { return mData; }
        QRectF boundingRect() const override;
    };

}
