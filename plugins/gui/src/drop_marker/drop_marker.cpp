#include "gui/drop_marker/drop_marker.h"

#include <QPropertyAnimation>

namespace hal
{
    DropMarker::DropMarker(Qt::Orientation orientation, QWidget* parent) : QFrame(parent), m_animation(new QPropertyAnimation(this)), m_orientation(orientation), m_fixed_width(0), m_fixed_height(0)
    {
        m_animation->setTargetObject(this);
        m_animation->setDuration(200);
        m_animation->setStartValue(0);
        m_animation->setEndValue(0);

        if (m_orientation == Qt::Horizontal)
            m_animation->setPropertyName("fixed_width");
        if (m_orientation == Qt::Vertical)
            m_animation->setPropertyName("fixed_height");
    }

    int DropMarker::fixed_width()
    {
        return m_fixed_width;
    }

    int DropMarker::fixed_height()
    {
        return m_fixed_height;
    }

    void DropMarker::set_fixed_width(int width)
    {
        m_fixed_width = width;

        setMinimumWidth(width);
        setMaximumWidth(width);
    }

    void DropMarker::set_fixed_height(int height)
    {
        m_fixed_height = height;

        setMinimumHeight(height);
        setMaximumHeight(height);
    }

    void DropMarker::set_end_value(int value)
    {
        m_animation->setEndValue(value);
    }

    void DropMarker::expand()
    {
        m_animation->setDirection(QPropertyAnimation::Forward);

        if (m_animation->state() == QAbstractAnimation::Stopped)
            m_animation->start();
    }

    void DropMarker::collapse()
    {
        m_animation->setDirection(QPropertyAnimation::Backward);

        if (m_animation->state() == QAbstractAnimation::Stopped)
            m_animation->start();
    }

    void DropMarker::reset()
    {
        if (m_animation->state() == QAbstractAnimation::Running)
            m_animation->stop();

        if (m_orientation == Qt::Horizontal)
            set_fixed_width(0);
        if (m_orientation == Qt::Vertical)
            set_fixed_height(0);
    }
}
