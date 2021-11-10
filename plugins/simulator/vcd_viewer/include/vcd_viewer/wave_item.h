#pragma once

#include <QGraphicsItem>
#include <QLine>

namespace hal {

    class WaveData;

    class WaveItem : public QGraphicsItem
    {
        int mWaveIndex;
        const WaveData* mData;
        QVector<QLine> mSolidLines;
        QVector<QLineF> mDotLines;

        float mMaxTime;
        bool mInactive;

        void construct();
    public:
        WaveItem(int iwave, const WaveData* dat);

        int waveIndex() const { return mWaveIndex; }
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        void setWavedata(const WaveData* dat);
        const WaveData* wavedata() const { return mData; }
        QRectF boundingRect() const override;
        float maxTime() const;
        void aboutToBeDeleted();
    };

}
