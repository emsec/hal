#include "vcd_viewer/wave_timescale.h"
#include "vcd_viewer/wave_graphics_view.h"
#include <QPainter>
#include <math.h>
#include <QPaintEvent>

namespace hal {

    WaveTimescale::WaveTimescale(QWidget *parent)
        : QWidget(parent), mXmag(1), mSceneLeft(0), mSceneRight(100), mWidth(100)
    {
        setFixedHeight(28);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    }

    void WaveTimescale::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setBrush(QColor::fromRgb(0xE0,0xE0,0xE0));
        painter.drawRect(rect());

        if (mXmag <= 0) return;
        int ilog = ceil(log(10/mXmag) / log(10));
        double minorTick = pow(10,ilog);
        int i = (int) floor(mSceneLeft/minorTick);
        for( ; ; i++)
        {
            double xScene = i * minorTick;
            if (xScene < mSceneLeft) continue;
            if (xScene > mSceneRight) break;
            double xPixel = (xScene - mSceneLeft) * mXmag;
            if (xPixel < 0) continue;
            if (xPixel > width()) break;
            if (i%5)
            {
                painter.setPen(QPen(QColor::fromRgb(0xA0,0xA0,0xA0),0));
                painter.drawLine(QLineF(xPixel,0,xPixel,10));
            }
            else
            {
                painter.setPen(QPen(QColor::fromRgb(0x70,0x70,0x70),0));
                painter.drawLine(QLineF(xPixel,0,xPixel,24));

                if (i%10 == 0)
                {
                    int ilabel = floor(xScene+0.5);
                    painter.setPen(QPen(Qt::black,0));
                    painter.drawText(xPixel,24,QString::number(ilabel));
                }
            }
        }
    }

    void WaveTimescale::setScale(float m11, int width, float scLeft, float scRight)
    {
        mXmag = m11;
        mSceneLeft = scLeft;
        mSceneRight = scRight;
        mWidth = width;
        WaveGraphicsView* wgv = dynamic_cast<WaveGraphicsView*>(parent());
        if (wgv) setFixedWidth(wgv->width());
        move(0,0);
        update();
    }

}
