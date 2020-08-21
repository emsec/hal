#include "code_editor/minimap_scrollbar.h"

#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

#include <cmath>
#include <math.h>

namespace hal
{
    MinimapScrollbar::MinimapScrollbar(QWidget* parent) : QWidget(parent),
          m_slider(new QFrame(this)),
          m_minimum(0),
          m_maximum(100),
          m_value(0),
          m_handle_length(0),
          m_handle_position(0),
          m_mouse_pressed(false),
          m_drag_offset(0),
          m_scrollbar(nullptr)
    {
        m_slider->setObjectName("slider");
        m_slider->show();
    }

    int MinimapScrollbar::minimum() const
    {
        return m_minimum;
    }

    int MinimapScrollbar::maximum() const
    {
        return m_maximum;
    }

    int MinimapScrollbar::value() const
    {
        return m_value;
    }

    int MinimapScrollbar::slider_height() const
    {
        return m_slider->height();
    }

    int MinimapScrollbar::slider_position() const
    {
        return m_slider->pos().y();
    }

    //void MinimapScrollbar::set_minimum(const int minimum)
    //{
    //    m_minimum = minimum;

    //    if (m_value < minimum)
    //        m_value = minimum;

    //    adjust_slider_to_value();
    //}

    //void MinimapScrollbar::set_maximum(const int maximum)
    //{
    //    m_maximum = maximum;

    //    if (m_value > maximum)
    //        m_value = maximum;

    //    adjust_slider_to_value();
    //}

    void MinimapScrollbar::set_range(const int minimum, const int maximum)
    {
        m_minimum = minimum;
        m_maximum = maximum;

        // MAYBE REDUNDANT, LEFT FOR COMPLETENESS
        if (m_value < minimum)
            m_value = minimum;

        else if (m_value > maximum)
            m_value = maximum;

        adjust_slider_to_value();
    }

    void MinimapScrollbar::set_value(const int value)
    {
        if (m_value != value)
        {
            if (value < m_minimum)
                m_value = m_minimum;

            else if (value > m_maximum)
                m_value = m_maximum;

            else
                m_value = value;

            adjust_slider_to_value();
        }
    }

    void MinimapScrollbar::set_slider_height(int height)
    {
        m_slider->resize(m_slider->width(), height);
        adjust_slider_to_value();
    }

    void MinimapScrollbar::set_scrollbar(QScrollBar* scrollbar)
    {
        m_scrollbar = scrollbar;
    }

    void MinimapScrollbar::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);
        // LEFT EMPTY INTENTIONALLY FOR TRANSPARENT BACKGROUND
    }

    void MinimapScrollbar::resizeEvent(QResizeEvent* event)
    {
        //QWidget::resizeEvent(event); // UNNECESSARY ?
        Q_UNUSED(event)

        m_slider->resize(width(), m_slider->height());
        adjust_slider_to_value();
    }

    void MinimapScrollbar::mousePressEvent(QMouseEvent* event)
    {
        if (m_slider->geometry().contains(event->pos()))
        {
            m_mouse_pressed = true;
            m_drag_offset   = event->pos().y() - m_slider->geometry().top();
            event->accept();
            return;
        }
        else
            event->ignore();
    }

    void MinimapScrollbar::mouseMoveEvent(QMouseEvent* event)
    {
        if (!m_scrollbar)
            return;

        if (m_mouse_pressed)
        {
            event->accept();    // UNSURE

            int y = event->pos().y() - m_drag_offset;

            if (y < 0)
                m_scrollbar->setValue(m_minimum);

            else if (y > height() - m_slider->height())
                m_scrollbar->setValue(m_maximum);

            else
            {
                int available_pixel = height() - m_slider->height();
                if (available_pixel <= 0)
                    return;

                int value_range = m_maximum - m_minimum;
                if (value_range <= 0)
                    return;

                double pixel_per_value = double(available_pixel) / double(value_range);
                m_scrollbar->setValue(std::round(y / pixel_per_value));
            }
        }
    }

    void MinimapScrollbar::mouseReleaseEvent(QMouseEvent* event)
    {
        m_mouse_pressed = false;
        event->accept();
    }

    void MinimapScrollbar::leaveEvent(QEvent* event)
    {
        m_mouse_pressed = false;
        QWidget::leaveEvent(event);
    }

    void MinimapScrollbar::adjust_slider_to_value()
    {
        int available_pixel = height() - m_slider->height();
        if (available_pixel <= 0)
            return;

        int value_range = m_maximum - m_minimum;
        if (value_range <= 0)
            return;

        double pixel_per_value = double(available_pixel) / double(value_range);
        int y                  = std::round(pixel_per_value * m_value);

        int max = height() - m_slider->height();
        if (y > max)
            y = max;

        m_slider->move(0, y);
    }
}
