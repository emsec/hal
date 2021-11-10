#include "vcd_viewer/wave_item.h"
#include "vcd_viewer/wave_scene.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QGraphicsScene>
#include <QPainter>
#include <math.h>

namespace hal {

    WaveItem::WaveItem(int iwave, const WaveData* dat)
        : mWaveIndex(iwave), mData(dat), mMaxTime(WaveScene::sMinSceneWidth), mInactive(false)
    {
        construct();
    }

    void WaveItem::construct()
    {
        prepareGeometryChange();
        mSolidLines.clear();
        mDotLines.clear();

        if (!childItems().isEmpty())
        {
            for (QGraphicsItem* cit : childItems())
                delete cit;
            childItems().clear();
        }

        if (!mData)
        {
            return;
        }

        auto lastIt = mData->constEnd();
        for (auto nextIt = mData->constBegin(); nextIt != mData->constEnd(); ++nextIt)
        {
            u64 t1 = nextIt.key();
            if (t1 > mMaxTime) mMaxTime = t1;
            if (lastIt != mData->constEnd())
            {
                u64 t0 = lastIt.key();
                if (lastIt.value() < 0)
                {
                    float ydot = 0.5;
                    mDotLines.append(QLineF(t0,ydot,t1,ydot));
                }
                else
                {
                    int lastVal = 1. - lastIt.value();
                    if (mData->bits()==1 || !lastIt.value())
                    {
                        mSolidLines.append(QLine(t0,lastVal,t1,lastVal)); // hline

                        if (nextIt.value() >= 0)
                        {
                            int nextVal = 1. - nextIt.value();
                            if (mData->bits()==1 || !nextIt.value())
                                mSolidLines.append(QLine(t1,lastVal,t1,nextVal)); // vline
                        }
                    }
                }
            }
            lastIt = nextIt;
        }

        if (mData->bits() > 1)
        {
            for (auto it = mData->constBegin(); it != mData->constEnd();)
            {
                u64 t0 = it.key();
                int v0 = it.value();
                ++it;
                if (v0 > 0)
                {
                    u64 t1 = it == mData->constEnd() ? mMaxTime : it.key();
                    QGraphicsSimpleTextItem* gti = new QGraphicsSimpleTextItem(QString::number(v0,16),this);
                    gti->setFlags(gti->flags() | QGraphicsItem::ItemIgnoresTransformations);
                    QFont font = gti->font();
                    font.setPixelSize(9);
                    gti->setFont(font);
                    gti->setPos( (t0+t1)/2. , 0);
                }
            }

        }
    }

    void WaveItem::aboutToBeDeleted()
    {
//        mMaxTime = 1;
//        prepareGeometryChange();
        mInactive = true;
    }

    float WaveItem::maxTime() const
    {
        float retval = WaveScene::sMinSceneWidth;
        if (wavedata()->maxTime() > retval) retval = wavedata()->maxTime();
        if (scene() && scene()->sceneRect().width() > retval) retval = scene()->sceneRect().width();
        return retval;
    }

    void WaveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        if (mInactive) return;

        float x1 = maxTime();
        if (x1 != mMaxTime)
        {
            mMaxTime = x1;
            if (mData->netType() == WaveData::ClockNet)
                construct();
            else
                prepareGeometryChange();
        }

        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(QPen(QBrush(Qt::darkBlue),0.,Qt::DotLine)); // TODO : style
        painter->drawLines(mDotLines);

        painter->setPen(QPen(QBrush(Qt::darkBlue),0.));  // TODO : style
        painter->drawLines(mSolidLines);

        float  y = 1;

        if (mData->isEmpty())
        {
            painter->setPen(QPen(QBrush(Qt::darkBlue),0.,Qt::DotLine));
            y -= 0.5;
            painter->drawLine(QLineF(0,y,x1,y));
            return;
        }

        if (mData->bits() > 1)
        {
            for (auto it = mData->constBegin(); it!=mData->constEnd();)
            {
                u64 t0 = it.key();
                int v0 = it.value();
                ++it;
                if (v0>0)
                {
                    u64 t1 = it==mData->constEnd() ? mMaxTime : it.key();
                    QRectF r(t0,-0.05,t1-t0,1.1);
                    painter->drawRoundedRect(r,50.0,0.5);
                }
            }
        }

        float x0 = mData->maxTime();
        if (x0 < x1)
        {
            if (mData->last() < 0)
            {
                y -= 0.5;
                painter->setPen(QPen(QBrush(Qt::cyan),0.,Qt::DotLine));
            }
            else
                y -= mData->last();
            painter->drawLine(QLineF(x0,y,x1,y));
        }
    }

    QRectF WaveItem::boundingRect() const
    {
        return QRectF(0,-0.05,mMaxTime,1.1);
    }

    void WaveItem::setWavedata(const WaveData *dat)
    {
        mData = dat;
        construct();
    }
}
