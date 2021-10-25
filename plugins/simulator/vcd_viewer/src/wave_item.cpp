#include <QPainter>

#include "vcd_viewer/wave_item.h"
#include "netlist_simulator_controller/wave_data.h"
#include <QGraphicsScene>
#include <QDebug>
#include <math.h>

namespace hal {

    WaveItem::WaveItem(const WaveData* dat, int off)
        : mData(dat), mYoffset(off), mMaxTime(1000)
    {
        if (!mData->empty() && mData->lastKey() > mMaxTime)
            mMaxTime = mData->lastKey();
        construct();
        qDebug() << "WaveItem:construct" << mData->id() << mData->name() << mMaxTime;
    }

    WaveItem::~WaveItem()
    {;}

    void WaveItem::construct()
    {
        if (!mData)
        {
            QRectF(0,mYoffset,1,1);
            return;
        }

        prepareGeometryChange();

        mSolidLines.clear();
        mDotLines.clear();
        auto lastIt = mData->constEnd();
        for (auto nextIt = mData->constBegin(); nextIt != mData->end(); ++nextIt)
        {
            if (lastIt != mData->constEnd())
            {
                if (lastIt.value() < 0)
                {
                    float ydot = mYoffset-0.5;
                    mDotLines.append(QLineF(lastIt.key(),ydot,nextIt.key(),ydot));
                }
                else
                {
                    int lastVal = mYoffset - lastIt.value();
                    mSolidLines.append(QLine(lastIt.key(),lastVal,nextIt.key(),lastVal)); // hline
                    if (nextIt.value() >= 0)
                    {
                        int nextVal = mYoffset - nextIt.value();
                        mSolidLines.append(QLine(nextIt.key(),lastVal,nextIt.key(),nextVal)); // vline
                    }
                }
            }
            lastIt = nextIt;
        }
    }

    void WaveItem::setMaxTime(float tmax)
    {
        if (!mData->isEmpty() && mData->lastKey() > tmax) return;
        prepareGeometryChange();
        mMaxTime = tmax > 1000 ? tmax : 1000;
        qDebug() << "WaveItem:setmaxtim" << mData->id() << mData->name() << mMaxTime;
        update();
    }

    void WaveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(QPen(QBrush(Qt::cyan),0.,Qt::DotLine)); // TODO : style
        painter->drawLines(mDotLines);

        painter->setPen(QPen(QBrush(Qt::cyan),0.));  // TODO : style
        painter->drawLines(mSolidLines);

        float  y = mYoffset;
        float x1 = mMaxTime;

        if (mData->isEmpty())
        {
            painter->setPen(QPen(QBrush(Qt::cyan),0.,Qt::DotLine));
            y -= 0.5;
            painter->drawLine(QLineF(0,y,x1,y));
            return;
        }

        float x0 = mData->lastKey();
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
        return QRectF(0,mYoffset-1.05,mMaxTime,1.1);
    }

    void WaveItem::setYoffset(int val)
    {
        mYoffset = val;
        construct();
    }

    void WaveItem::setWavedata(const WaveData *dat)
    {
        mData = dat;
        construct();
    }
}
