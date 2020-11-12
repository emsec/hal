#include "gui/dialogs/warning_dialog.h"

#include <QPainter>
#include <QPropertyAnimation>

namespace hal
{
    WarningDialog::WarningDialog(QWidget* parent, bool animate) : Dialog(parent),
        mLineAnimation(new QPropertyAnimation(this, "lineOffset", this)),
        mLineOffset(0),
        mBarHeight(16),
        mLineWidth(10),
        mLineGradient(10),
        mLineSpacing(10),
        mAnimate(animate),
        mDirection(direction::right)
    {
        mLineAnimation->setDuration(700);
        mLineAnimation->setStartValue(0);
        mLineAnimation->setEndValue(mLineWidth + mLineSpacing);
        mLineAnimation->setLoopCount(-1);
        mLineAnimation->setEasingCurve(QEasingCurve::Linear);

        QPointF point(mLineWidth, 0);

        mLeftLeaningLine.lineTo(point);
        point.setX(mLineWidth + mLineGradient);
        point.setY(mBarHeight);
        mLeftLeaningLine.lineTo(point);
        point.setX(mLineGradient);
        mLeftLeaningLine.lineTo(point);
        mLeftLeaningLine.closeSubpath();

        point.setX(mLineWidth);
        point.setY(0);

        mRightLeaningLine.lineTo(point);
        point.setX(mLineWidth - mLineGradient);
        point.setY(mBarHeight);
        mRightLeaningLine.lineTo(point);
        point.setX(-mLineGradient);
        mRightLeaningLine.lineTo(point);
        mRightLeaningLine.closeSubpath();
    }

    void WarningDialog::fadeInStarted()
    {
        if (mAnimate)
            mLineAnimation->start();
    }

    void WarningDialog::fadeInFinished()
    {
        // DELETE AFTER TESTING
    }

    void WarningDialog::setDirection(const direction d)
    {
        mDirection = d;
    }

    int WarningDialog::lineOffset() const
    {
        return mLineOffset;
    }

    void WarningDialog::setLineOffset(const int offset)
    {
        mLineOffset = offset;
        update();
    }

    void WarningDialog::paintEvent(QPaintEvent* event)
    {
        QFrame::paintEvent(event);

        QPainter painter(this);

        QPen pen;
        pen.setCosmetic(true);
        //pen.setColor(QColor(204, 184, 0));
        //pen.setColor(QColor(100, 100, 100));
        pen.setColor(QColor(50, 50, 50));
        painter.setPen(pen);

        painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

        //painter.fillRect(QRect(1, 1, width() - 2, 69), QBrush(QColor(9, 11, 13), Qt::Dense5Pattern));
        painter.fillRect(QRect(1, 1, width() - 2, 69), QBrush(QColor(77, 77, 0), Qt::Dense3Pattern));
        //painter.fillRect(QRect(1, 1, width() - 2, 69), QBrush(QColor(204, 184, 0)));
        painter.fillRect(QRect(1, 70, width() - 2, height() -71), QBrush(QColor(14, 16, 18)));

        pen.setColor(QColor(204, 184, 0));
        painter.setPen(pen);
        painter.drawRect(QRect(1, 1, width() - 2, 69));

        painter.setClipRect(QRect(0, 0, width(), height()));
        painter.setRenderHint(QPainter::Antialiasing, true);

        QFont font;
        font.setPixelSize(24);
        painter.setFont(font);

        //pen.setColor("#A9B7C6");
        pen.setColor("#FFFFFF");
        //pen.setColor(QColor(9, 11, 13));
        painter.setPen(pen);
        painter.drawText(QRectF(1, 1, width() - 1, 69), Qt::AlignCenter, "Warning: Something requires your attention");

        painter.translate(0, 75);

        const int stride = mLineWidth + mLineSpacing;

        if (mDirection == direction::left)
        {
            painter.translate(-mLineOffset, 0);

            for (int x = 0; x <= width(); x += stride)
            {
                painter.fillPath(mLeftLeaningLine, QColor(255, 230, 0));
                painter.translate(stride, 0);
            }
        }
        else
        {
            painter.translate(mLineOffset - mLineWidth, 0);

            for (int x = 0; x <= width(); x += stride)
            {
                painter.fillPath(mRightLeaningLine, QColor(255, 230, 0));
                painter.translate(stride, 0);
            }
        }
    }

    QSize WarningDialog::sizeHint() const
    {
        //DEBUG CODE
        return QSize(600, 400);
    }
}
