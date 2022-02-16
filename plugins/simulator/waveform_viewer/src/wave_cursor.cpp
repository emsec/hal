#include "waveform_viewer/wave_cursor.h"
#include "waveform_viewer/wave_scrollbar.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QPalette>

namespace hal {

    WaveCursor::WaveCursor(const WaveTransform* trans, const WaveScrollbar* scroll, QWidget* parent)
        : QWidget(parent), mTransform(trans), mScrollbar(scroll), mPosition(100,400)
    {
        setWindowOpacity(0);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        setStyleSheet("background: transparent;");
        setFixedWidth(sWidth);
        setCursorPosition(mPosition);
    }

    void WaveCursor::paintEvent(QPaintEvent *event)
    {
        QPainter paint(this);

//        QBrush transparent(QColor::fromRgb(255,255,255,40));
//        paint.fillRect(QRectF(0,0,81,height()),transparent);
        paint.setRenderHint(QPainter::Antialiasing,true);
        paint.setPen(QPen(QBrush(Qt::red),0));
        paint.drawLine(sWidth/2,0,sWidth/2,height());

        int y0 = mPosition.y()-sLabHeight/2;
        if (y0 < sLabHeight) y0 = sLabHeight;
        if (y0 + sLabHeight + 1 > height()) y0 = height() - sLabHeight - 1;
        QRect rectTvalue(0,y0,sWidth,sLabHeight);

        paint.setBrush(Qt::black);
        paint.setPen(QPen(QBrush(Qt::red),0));
        paint.drawRoundedRect(rectTvalue, 15, 10, Qt::AbsoluteSize);
        paint.setPen(QPen(QColor("#c0c0c0")));
        QFont font = paint.font();
        font.setBold(true);
        font.setPointSizeF(10);
        paint.setFont(font);

        double tval = mScrollbar->tPos(mPosition.x());
        paint.drawText(rectTvalue, QString::number(tval,'f',0), Qt::AlignHCenter | Qt::AlignVCenter);
    }

    void WaveCursor::setViewportHeight(int height)
    {
        setFixedHeight(height);
        update();
    }

    QRect WaveCursor::labelRect() const
    {
        return QRect(mPosition.x()-sWidth/2,mPosition.y()-sLabHeight/2,sWidth,sLabHeight);
    }

    void WaveCursor::setCursorPosition(const QPoint& pos)
    {
        mPosition = pos;
        move(pos.x()-sWidth/2,0);
        update();
    }
}
