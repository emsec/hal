#pragma once

#include <QGraphicsItem>
#include <QLine>

namespace hal {

    class WaveData;

    class WaveItem : public QGraphicsItem
    {
        const WaveData* mData;
        QRectF mRect;
        QVector<QLine> mSolidLines;
        QVector<QLineF> mDotLines;

        int mYoffset;

        void construct();
    public:
        WaveItem(const WaveData* dat, int off);
        ~WaveItem();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        void setYoffset(int val);
        void setWavedata(const WaveData* dat);
        void setSceneMax(float xmax);
        const WaveData* wavedata() const { return mData; }
        QRectF boundingRect() const override;
    };

}
