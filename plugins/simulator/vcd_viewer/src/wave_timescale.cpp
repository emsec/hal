#include "vcd_viewer/wave_timescale.h"
#include <math.h>
#include <QPainter>
#include <QDebug>
#include <QWidget>

namespace hal {

    WaveTimescale::WaveTimescale(int max)
        : mMinorTicDistance(10), mMaxValue(max), mXmag(1)
    {;}

    void WaveTimescale::xScaleChanged(float mag)
    {
        mXmag = mag;
        int n = floor(log10(100./mXmag));
        mMinorTicDistance = 1;
        for (int i=0; i<n; i++)
            mMinorTicDistance *= 10;
    }

    void WaveTimescale::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget* widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        int count = 0;
        QPen pen = painter->pen();
        for (int x=0; x<mMaxValue; x+=mMinorTicDistance)
            if (count++ % 10 == 0)
            {
                /*
                if (mXmag > 0)
                {
                    pen.setColor(Qt::gray);
                    pen.setWidth(5./mXmag);
                    pen.setCosmetic(false);
                }
                else
                {
                    */
                pen.setColor(QColor("#c0c0c0")); // TODO : style
                pen.setWidth(0);
                pen.setCosmetic(true);
                // }
                painter->setPen(pen);
                painter->drawLine(x,0,x,2);
            }
            else
            {
                pen.setColor(QColor("#a0a0a0")); // TODO : style
                pen.setWidth(0);
                pen.setCosmetic(true);
                painter->setPen(pen);
                painter->drawLine(x,0,x,1);
            }
        int labelDistance = 10;
        int closerDistance[3] = {5, 2, 1};
        for (int i=0; i<3; i++)
            if ( closerDistance[i] * mXmag * mMinorTicDistance > 200 )
                labelDistance = closerDistance[i];
            else break;

        for (int x=0; x<mMaxValue; x+= labelDistance*mMinorTicDistance)
        {
            pen.setColor(QColor("#c0c0c0")); // TODO : style
            QFont font = painter->font();
            font.setBold(true);
            font.setPointSizeF(1.5);
            painter->setFont(font);
            QTransform trans = painter->transform();
            painter->setTransform(QTransform(15./mXmag,0,0,1.,x,4.2),true);
            painter->drawText(QPointF(0,0),QString::number(x));
            painter->setTransform(trans);
        }
    }

    QRectF WaveTimescale::boundingRect() const
    {
        return QRectF(0,0,mMaxValue,10);
    }
}
