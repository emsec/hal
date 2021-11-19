#include "vcd_viewer/wave_timescale.h"
#include <QPainter>
#include <math.h>
#include <QDebug>
#include <QPaintEvent>

namespace hal {

    WaveTimescale::WaveTimescale(QWidget *parent)
        : QWidget(parent), mXmag(1), mWidth(100), mXscrollValue(0)
    {
        setFixedHeight(28);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    }

    void WaveTimescale::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
//        qDebug() << "repaint" << event->rect();
        QPainter painter(this);
        painter.setBrush(QColor::fromRgb(0xE0,0xE0,0xE0));
        painter.drawRect(rect());

        if (mXmag <= 0) return;
        int ilog = ceil(log(10/mXmag) / log(10));
        double minorTick = pow(10,ilog);
        float x;
        for(int i = 0; (x = i*minorTick*mXmag-mXscrollValue) <= width(); i++)
        {
            if (x<0) continue;
            if (x > width()) break;
            if (i%5)
            {
                painter.setPen(QPen(QColor::fromRgb(0xA0,0xA0,0xA0),0));
                painter.drawLine(QLineF(x,0,x,10));
            }
            else
            {
                painter.setPen(QPen(QColor::fromRgb(0x70,0x70,0x70),0));
                painter.drawLine(QLineF(x,0,x,24));

                if (i%10 == 0)
                {
                    int ilabel = floor(i*minorTick+0.5);
                    painter.setPen(QPen(Qt::black,0));
                    painter.drawText(x,24,QString::number(ilabel));
                }
            }
        }
    }

    void WaveTimescale::setScale(float m11, float scWidth, int xScrollValue)
    {
        mXmag = m11;
        mWidth = floor(scWidth * m11 + 0.5);
        mXscrollValue = xScrollValue;
        if (mWidth < 160000)
            setFixedWidth(mWidth);
        move(0,0);
        update();
    }

}
