#include "vcd_viewer/wave_label.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace hal {

    QPixmap* WaveLabel::sXdelete = nullptr;

    WaveLabel::WaveLabel(int inx, const QString &nam, QWidget *parent)
        : QWidget(parent), mDataIndex(inx), mName(nam), mValue(0)
    {;}

    QPixmap* WaveLabel::piXdelete()
    {
        if (!sXdelete) sXdelete = new QPixmap(":/icons/x_delete", "PNG");
        return sXdelete;
    }

    void WaveLabel::setValue(int val)
    {
        if (val==mValue) return;
        mValue = val;
        update();
    }

    void WaveLabel::mouseDoubleClickEvent(QMouseEvent *event)
    {
        Q_UNUSED(event);
        Q_EMIT doubleClicked(mDataIndex);
    }

    void WaveLabel::mousePressEvent(QMouseEvent *event)
    {
        mMousePoint = event->pos();
    }

    void WaveLabel::mouseReleaseEvent(QMouseEvent *event)
    {
        QPoint delta = event->pos() - mMousePoint;
        if (abs(delta.x())<2 && abs(delta.y())<2)
            if (mDeleteRect.contains(mMousePoint))
                Q_EMIT triggerDelete(mDataIndex);
        mMousePoint.setX(-99999);
    }

    QBrush WaveLabel::valueBackground() const
    {
        if (!mValue) return QBrush(Qt::white);
        if (mValue>0) return QBrush(Qt::green);
        return QBrush(Qt::lightGray);
    }

    QString WaveLabel::valueString() const
    {
        switch (mValue)
        {
        case -1 : return "x";
        case -2 : return "z";
        }
        return QString::number(mValue);
    }

    void WaveLabel::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter paint(this);
        paint.setRenderHint(QPainter::Antialiasing,true);
        paint.setRenderHint(QPainter::TextAntialiasing,true);
        paint.fillRect(rect(),QBrush(QColor("#C0102040"))); // TODO : style
        paint.setPen(QPen(Qt::gray,0.3));
        paint.drawRoundedRect(rect(),15.,15.);
        QFont font = paint.font();
        font.setPointSize(14);
        font.setBold(true);
        paint.setFont(font);
        paint.setPen(QPen(Qt::white,0));
        paint.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, mName);
        float h = height();
        float bm = 0.12;
        QRectF rBullet(bm*h,bm*h,(1-2*bm)*h,(1-2*bm)*h);
        paint.setPen(QPen(Qt::black,0.4));
        paint.setBrush(valueBackground());
        paint.drawEllipse(rBullet);

        font.setPointSize(10);
        paint.setFont(font);
        paint.drawText(rBullet, Qt::AlignHCenter | Qt::AlignVCenter, valueString());
        float dm = 0.25;
        mDeleteRect = QRectF(width()+(2*dm-1)*h,0,(1-2*dm)*h,(1-2*dm)*h);
        QPixmap* pix = piXdelete();
        paint.drawPixmap(mDeleteRect, QPixmap(*pix), pix->rect());
    }

}
