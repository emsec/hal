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
        bool mInactive;

        void construct();
    public:
        WaveItem(const WaveData* dat, int off);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        void setYoffset(int val);
        void setWavedata(const WaveData* dat);
        const WaveData* wavedata() const { return mData; }
        QRectF boundingRect() const override;
        float maxTime() const;
        void aboutToBeDeleted();
    };

}
