#include "gui/dialogs/success_dialog.h"

#include <QPainter>
#include <QPropertyAnimation>

namespace hal
{
    SuccessDialog::SuccessDialog(QWidget* parent) : Dialog(parent),
        mArrowAnimation(new QPropertyAnimation(this, "arrowOffset", this)),
        mArrowOffset(0),
        mBarHeight(30),
        mArrowWidth(10),
        mArrowLength(30),
        mDirection(direction::left)
    {
        mArrowAnimation->setDuration(700);
        mArrowAnimation->setStartValue(0);
        mArrowAnimation->setEndValue(100);
        mArrowAnimation->setLoopCount(-1);
        mArrowAnimation->setEasingCurve(QEasingCurve::Linear);

        mArrowLeft.lineTo(QPointF(-(mArrowLength - mArrowWidth), (mBarHeight / 2)));
        mArrowLeft.lineTo(QPointF(0, mBarHeight));
        mArrowLeft.lineTo(QPointF(-mArrowWidth, mBarHeight));
        mArrowLeft.lineTo(QPointF(-mArrowLength, (mBarHeight / 2)));
        mArrowLeft.lineTo(QPointF(-mArrowWidth, 0));
        mArrowLeft.closeSubpath();

        mArrowRight.lineTo(QPointF((mArrowLength - mArrowWidth), (mBarHeight / 2)));
        mArrowRight.lineTo(QPointF(0, mBarHeight));
        mArrowRight.lineTo(QPointF(mArrowWidth, mBarHeight));
        mArrowRight.lineTo(QPointF(mArrowLength, (mBarHeight / 2)));
        mArrowRight.lineTo(QPointF(mArrowWidth, 0));
        mArrowRight.closeSubpath();
    }

    void SuccessDialog::fadeInStarted()
    {
        start();
    }

    void SuccessDialog::fadeInFinished()
    {
        // DELETE AFTER TESTING
    }

    void SuccessDialog::start()
    {
        mArrowAnimation->start();
    }

    void SuccessDialog::stop()
    {
        hide();
        mArrowAnimation->stop();
    }

    void SuccessDialog::setDirection(const direction d)
    {
        mDirection = d;
    }

    int SuccessDialog::arrowOffset() const
    {
        return mArrowOffset;
    }

    void SuccessDialog::setArrowOffset(const int offset)
    {
        mArrowOffset = offset;
        update();
    }

    void SuccessDialog::paintEvent(QPaintEvent* event)
    {
        QFrame::paintEvent(event);

        QPainter painter(this);

        QPen pen;
        pen.setCosmetic(true);
        pen.setColor(QColor(85, 255, 0));
        painter.setPen(pen);

        painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

        painter.setClipRect(QRect(0, 0, width(), height()));
        painter.setRenderHint(QPainter::Antialiasing, true);

        //painter.drawText(QRectF(10, 40, 50, 50), QString::number(mArrowOffset));

        if (mDirection == direction::left)
        {
            qreal value = (40.0 / 100.0) * mArrowOffset;
            painter.translate(-value, 0);

            for (int x = 0; x < width(); x += 10)
            {
                painter.fillPath(mArrowLeft, QColor(85, 255, 0));
                painter.translate(20, 0);
            }
        }
        else
        {
            qreal value = (40.0 / 100.0) * mArrowOffset;
            painter.translate(-40, 0);
            painter.translate(value, 0);

            for (int x = 0; x < width(); x += 10)
            {
                painter.fillPath(mArrowRight, QColor(85, 255, 0));
                painter.translate(20, 0);
            }
        }
    }

    QSize SuccessDialog::sizeHint() const
    {
        //DEBUG CODE
        return QSize(600, 400);
    }
}
