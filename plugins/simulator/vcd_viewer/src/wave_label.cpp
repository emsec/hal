#include "vcd_viewer/wave_label.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace hal {

    QPixmap* WaveLabel::sXdelete = nullptr;

    const char* WaveLabel::sStateColor[3] = {"#707071", "#102080", "#802010"};

    WaveLabel::WaveLabel(int inx, const QString &nam, QWidget *parent)
        : QWidget(parent), mDataIndex(inx), mName(nam), mValue(0), mState(0), mHighlight(false), mGhostShape(nullptr)
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

    void WaveLabel::setHighlight(bool hl)
    {
        if (hl == mHighlight) return;
        mHighlight = hl;
        update();
    }

    void WaveLabel::mousePressEvent(QMouseEvent *event)
    {
        mMousePoint = event->pos();
        mMouseRelative = mMousePoint - pos();
        update();
    }

    void WaveLabel::mouseMoveEvent(QMouseEvent *event)
    {
        QPoint delta = event->pos() - mMousePoint;
        if (abs(delta.x())>=2 && abs(delta.y())>=2)
        {
            mState = 1;
            QWidget* w = static_cast<QWidget*>(parent());
            if (!mGhostShape)
            {
                mGhostShape = new WaveLabel(-1, mName, w);
                mGhostShape->mValue = mValue;
                mGhostShape->mState = 3;
                mGhostShape->setFixedWidth(width());
                mGhostShape->show();
            }
            QPoint gpos = event->pos()-mMouseRelative;
            mGhostShape->move(gpos);
            Q_EMIT triggerMove(mDataIndex, gpos.y());
            update();
        }
    }

    void WaveLabel::mouseReleaseEvent(QMouseEvent *event)
    {
        if (mGhostShape)
        {
            WaveLabel* del = mGhostShape;
            mGhostShape = nullptr;
            del->deleteLater();
        }
        mState = 0;
        QPoint delta = event->pos() - mMousePoint;
        if (abs(delta.x())<2 && abs(delta.y())<2)
        {
            if (mDeleteRect.contains(mMousePoint))
                Q_EMIT triggerDelete(mDataIndex);
        }
        else if (abs(delta.y())>height())
            Q_EMIT triggerSwap(mDataIndex, (event->pos()-mMouseRelative).y());
        mMousePoint = QPoint(-99999,0);
    }

    QBrush WaveLabel::valueBackground() const
    {
        int inx = mValue < -1 || mValue > 1 ? 0 : mValue + 1;
        return QBrush(QColor(sStateColor[inx]));
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
        QColor bgcol("#206080");
        switch (mState)
        {
        case 0: bgcol = mHighlight ? QColor("#108080") : QColor("#102040"); break;
        case 1: bgcol.setAlpha(255); break;
        case 2: bgcol = QColor("#108040"); break;
        case 3: bgcol.setAlpha(80); break;
        }

        paint.fillRect(rect(),QBrush(bgcol)); // TODO : style
        paint.setPen(QPen(Qt::gray,0.3));
        paint.drawRoundedRect(rect(),15.,15.);
        QFont font = paint.font();
        font.setPointSize(14);
        font.setBold(true);
        paint.setFont(font);
        paint.setPen(QPen(Qt::white,0));
        paint.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, mName);

        if (mState >= 3) return;
        float h = height();
        float bm = 0.12;
        QRectF rBullet(bm*h,bm*h,(1-2*bm)*h,(1-2*bm)*h);
        paint.setPen(QPen(Qt::black,0.4));
        paint.setBrush(valueBackground());
        paint.drawEllipse(rBullet);

        font.setPointSize(10);
        paint.setFont(font);
        paint.setPen(QPen(Qt::white,0));
        paint.drawText(rBullet, Qt::AlignHCenter | Qt::AlignVCenter, valueString());
        paint.setPen(QPen(Qt::black,0.4));
        float dm = 0.25;
        mDeleteRect = QRectF(width()+(2*dm-1)*h,0,(1-2*dm)*h,(1-2*dm)*h);
        QPixmap* pix = piXdelete();
        paint.drawPixmap(mDeleteRect, QPixmap(*pix), pix->rect());
    }

}
