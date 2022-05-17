#include "waveform_viewer/wave_timescale.h"
#include "waveform_viewer/wave_transform.h"
#include "waveform_viewer/wave_scrollbar.h"
#include <QPainter>
#include <math.h>
#include <QPaintEvent>

namespace hal {

    WaveTimescale::WaveTimescale(const WaveTransform *trans, const WaveScrollbar *scroll, QWidget *parent)
        : QWidget(parent), mTransform(trans), mScrollbar(scroll)
    {
        setFixedHeight(28);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        move(0,0);
    }

    void WaveTimescale::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setBrush(QColor::fromRgb(0xE0,0xE0,0xE0));
        painter.drawRect(rect());

        if (mTransform->scale() <= 0) return;
        WaveLogicTimescale wlts(mTransform->scale());

        // scale
        for (quint64 tMinor = ceil(mScrollbar->tLeftI() / wlts.minorInterval()) * wlts.minorInterval();
             tMinor<=mTransform->tMax(); tMinor += wlts.minorInterval())
        {
            float x = mScrollbar->xPosF(tMinor);
            if (x > event->rect().width()) break;
            if (tMinor%wlts.majorInterval() == 0)
            {
                // major tick
                painter.setPen(QPen(QColor("#707070"),0));
                painter.drawLine(QLineF(x,0,x,tMinor%wlts.labelInterval() == 0 ? 24 : 12));
            }
            else
            {
                // minor tick
                painter.setPen(QPen(QColor("#a0a0a0"),0));
                painter.drawLine(QLineF(x,0,x,10));
            }
            if (tMinor%wlts.labelInterval() == 0)
            {
                // label
                painter.setPen(QPen(Qt::black,0));
                painter.drawText(x+1,24,QString::number(tMinor));
            }
        }
    }

    void WaveTimescale::setScale(int viewportWidth)
    {
        if (viewportWidth > 1) setFixedWidth(viewportWidth);
        move(0,0);
        update();
    }

    WaveLogicTimescale::WaveLogicTimescale(double scale)
    {
        int z = floor((2.-log(scale) / log(10)) * 3);
        int exp = z<3? 1 : z/3;
        quint64 base = floor(pow(10,exp)+0.5);
        switch (z%3)
        {
        case 0:
            mTlabel = z<3 ? base/2 : 2*base;
            mTmajor = base;
            mTminor = base/10;
            break;
        case 1:
            mTlabel = z<3 ? base : 5*base;
            mTmajor = base;
            mTminor = base/5;
            break;
        case 2:
            mTlabel = z<3 ? 2*base : 10*base;
            mTmajor = 10*base;
            mTminor = base;
            break;
        }
    }
}
