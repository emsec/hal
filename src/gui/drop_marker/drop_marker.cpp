#include "drop_marker/drop_marker.h"

#include <QPropertyAnimation>
namespace hal{
drop_marker::drop_marker(Qt::Orientation orientation, QWidget* parent) : QFrame(parent), m_animation(new QPropertyAnimation(this)), m_orientation(orientation), m_fixed_width(0), m_fixed_height(0)
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

int drop_marker::fixed_width()
{
    return m_fixed_width;
}

int drop_marker::fixed_height()
{
    return m_fixed_height;
}

void drop_marker::set_fixed_width(int width)
{
    m_fixed_width = width;

    setMinimumWidth(width);
    setMaximumWidth(width);
}

void drop_marker::set_fixed_height(int height)
{
    m_fixed_height = height;

    setMinimumHeight(height);
    setMaximumHeight(height);
}

void drop_marker::set_end_value(int value)
{
    m_animation->setEndValue(value);
}

void drop_marker::expand()
{
    m_animation->setDirection(QPropertyAnimation::Forward);

    if (m_animation->state() == QAbstractAnimation::Stopped)
        m_animation->start();
}

void drop_marker::collapse()
{
    m_animation->setDirection(QPropertyAnimation::Backward);

    if (m_animation->state() == QAbstractAnimation::Stopped)
        m_animation->start();
}

void drop_marker::reset()
{
    if (m_animation->state() == QAbstractAnimation::Running)
        m_animation->stop();

    if (m_orientation == Qt::Horizontal)
        set_fixed_width(0);
    if (m_orientation == Qt::Vertical)
        set_fixed_height(0);
}
}
