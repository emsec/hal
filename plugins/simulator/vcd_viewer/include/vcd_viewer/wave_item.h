#pragma once

#include <QGraphicsItem>
#include <QLine>
#include "netlist_simulator_controller/wave_data.h"

namespace hal {

    class WaveValueAsTextItem : public QGraphicsItem
    {
        QString mText;
        float mWidth;
        float mXmag;
    public:
        WaveValueAsTextItem(const QString& txt, float w, float m11, QGraphicsItem* parentItem = nullptr);
        void updateScaleFactor(float m11);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        QRectF boundingRect() const override;
    };

    class WaveItem : public QGraphicsItem, public WaveGraphicsItem
    {
        int mWaveIndex;
        WaveData* mData;
        QVector<QLine> mSolidLines;
        QVector<QLineF> mDotLines;

        float mMaxTime;
        bool mInactive;

        void construct();

        static const char* sLineColor[];  // 0=value(solid)  1=X(dotted)
    public:
        WaveItem(int iwave, WaveData* dat);
        ~WaveItem();

        int waveIndex() const { return mWaveIndex; }
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        const WaveData* wavedata() const { return mData; }
        QRectF boundingRect() const override;
        float maxTime() const;
        void aboutToBeDeleted();
        void updateScaleFactor(float m11);
        void updateGraphicsItem(WaveData* wd) override;
        void removeGraphicsItem() override;
        void setItemVisible(bool vis) override;
    };

}
