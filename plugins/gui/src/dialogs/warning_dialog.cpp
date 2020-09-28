#include "gui/dialogs/warning_dialog.h"

#include <QPainter>
#include <QPropertyAnimation>

namespace hal
{
    WarningDialog::WarningDialog(QWidget* parent, bool animate) : Dialog(parent),
        m_line_animation(new QPropertyAnimation(this, "line_offset", this)),
        m_line_offset(0),
        m_bar_height(16),
        m_line_width(10),
        m_line_gradient(10),
        m_line_spacing(10),
        m_animate(animate),
        m_direction(direction::right)
    {
        m_line_animation->setDuration(700);
        m_line_animation->setStartValue(0);
        m_line_animation->setEndValue(m_line_width + m_line_spacing);
        m_line_animation->setLoopCount(-1);
        m_line_animation->setEasingCurve(QEasingCurve::Linear);

        QPointF point(m_line_width, 0);

        m_left_leaning_line.lineTo(point);
        point.setX(m_line_width + m_line_gradient);
        point.setY(m_bar_height);
        m_left_leaning_line.lineTo(point);
        point.setX(m_line_gradient);
        m_left_leaning_line.lineTo(point);
        m_left_leaning_line.closeSubpath();

        point.setX(m_line_width);
        point.setY(0);

        m_right_leaning_line.lineTo(point);
        point.setX(m_line_width - m_line_gradient);
        point.setY(m_bar_height);
        m_right_leaning_line.lineTo(point);
        point.setX(-m_line_gradient);
        m_right_leaning_line.lineTo(point);
        m_right_leaning_line.closeSubpath();
    }

    void WarningDialog::fade_in_started()
    {
        if (m_animate)
            m_line_animation->start();
    }

    void WarningDialog::fade_in_finished()
    {
        // DELETE AFTER TESTING
    }

    void WarningDialog::set_direction(const direction d)
    {
        m_direction = d;
    }

    int WarningDialog::line_offset() const
    {
        return m_line_offset;
    }

    void WarningDialog::set_line_offset(const int offset)
    {
        m_line_offset = offset;
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

        const int stride = m_line_width + m_line_spacing;

        if (m_direction == direction::left)
        {
            painter.translate(-m_line_offset, 0);

            for (int x = 0; x <= width(); x += stride)
            {
                painter.fillPath(m_left_leaning_line, QColor(255, 230, 0));
                painter.translate(stride, 0);
            }
        }
        else
        {
            painter.translate(m_line_offset - m_line_width, 0);

            for (int x = 0; x <= width(); x += stride)
            {
                painter.fillPath(m_right_leaning_line, QColor(255, 230, 0));
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
