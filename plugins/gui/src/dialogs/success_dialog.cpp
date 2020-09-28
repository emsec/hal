#include "gui/dialogs/success_dialog.h"

#include <QPainter>
#include <QPropertyAnimation>

namespace hal
{
    SuccessDialog::SuccessDialog(QWidget* parent) : Dialog(parent),
        m_arrow_animation(new QPropertyAnimation(this, "arrow_offset", this)),
        m_arrow_offset(0),
        m_bar_height(30),
        m_arrow_width(10),
        m_arrow_length(30),
        m_direction(direction::left)
    {
        m_arrow_animation->setDuration(700);
        m_arrow_animation->setStartValue(0);
        m_arrow_animation->setEndValue(100);
        m_arrow_animation->setLoopCount(-1);
        m_arrow_animation->setEasingCurve(QEasingCurve::Linear);

        m_arrow_left.lineTo(QPointF(-(m_arrow_length - m_arrow_width), (m_bar_height / 2)));
        m_arrow_left.lineTo(QPointF(0, m_bar_height));
        m_arrow_left.lineTo(QPointF(-m_arrow_width, m_bar_height));
        m_arrow_left.lineTo(QPointF(-m_arrow_length, (m_bar_height / 2)));
        m_arrow_left.lineTo(QPointF(-m_arrow_width, 0));
        m_arrow_left.closeSubpath();

        m_arrow_right.lineTo(QPointF((m_arrow_length - m_arrow_width), (m_bar_height / 2)));
        m_arrow_right.lineTo(QPointF(0, m_bar_height));
        m_arrow_right.lineTo(QPointF(m_arrow_width, m_bar_height));
        m_arrow_right.lineTo(QPointF(m_arrow_length, (m_bar_height / 2)));
        m_arrow_right.lineTo(QPointF(m_arrow_width, 0));
        m_arrow_right.closeSubpath();
    }

    void SuccessDialog::fade_in_started()
    {
        start();
    }

    void SuccessDialog::fade_in_finished()
    {
        // DELETE AFTER TESTING
    }

    void SuccessDialog::start()
    {
        m_arrow_animation->start();
    }

    void SuccessDialog::stop()
    {
        hide();
        m_arrow_animation->stop();
    }

    void SuccessDialog::set_direction(const direction d)
    {
        m_direction = d;
    }

    int SuccessDialog::arrow_offset() const
    {
        return m_arrow_offset;
    }

    void SuccessDialog::set_arrow_offset(const int offset)
    {
        m_arrow_offset = offset;
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

        //painter.drawText(QRectF(10, 40, 50, 50), QString::number(m_arrow_offset));

        if (m_direction == direction::left)
        {
            qreal value = (40.0 / 100.0) * m_arrow_offset;
            painter.translate(-value, 0);

            for (int x = 0; x < width(); x += 10)
            {
                painter.fillPath(m_arrow_left, QColor(85, 255, 0));
                painter.translate(20, 0);
            }
        }
        else
        {
            qreal value = (40.0 / 100.0) * m_arrow_offset;
            painter.translate(-40, 0);
            painter.translate(value, 0);

            for (int x = 0; x < width(); x += 10)
            {
                painter.fillPath(m_arrow_right, QColor(85, 255, 0));
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
