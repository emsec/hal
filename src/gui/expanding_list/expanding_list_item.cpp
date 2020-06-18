#include "expanding_list/expanding_list_item.h"

#include "expanding_list/expanding_list_button.h"

#include <QPropertyAnimation>
#include <QStyle>
namespace hal{
expanding_list_item::expanding_list_item(expanding_list_button* parent_button, QWidget* parent)
    : QFrame(parent), m_parent_button(parent_button), m_collapsed_height(0), m_expanded_height(0),
      //m_animation(new QPropertyAnimation(this, "minimumHeight", this)),
      m_animation(new QPropertyAnimation(this, "fixed_height", this)), m_expanded(false), m_fixed_height(m_parent_button->minimumHeight())
{
    parent_button->setParent(this);
    parent_button->show();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(m_parent_button->minimumHeight());
}

QSize expanding_list_item::minimumSizeHint() const
{
    return minimumSize();
}

QSize expanding_list_item::sizeHint() const
{
    return size();
}

void expanding_list_item::resizeEvent(QResizeEvent* event)
{
    int width = event->size().width();

    m_parent_button->resize(width, m_parent_button->height());

    for (expanding_list_button* button : m_child_buttons)
        button->resize(width, button->height());
}

bool expanding_list_item::expanded()
{
    return m_expanded;
}

int expanding_list_item::fixed_height()
{
    return m_fixed_height;
}

bool expanding_list_item::contains(expanding_list_button* button)
{
    if (button == m_parent_button)
        return true;

    for (expanding_list_button* child_button : m_child_buttons)
        if (button == child_button)
            return true;

    return false;
}

expanding_list_button* expanding_list_item::parent_button()
{
    return m_parent_button;
}

void expanding_list_item::append_child_button(expanding_list_button* button)
{
    m_child_buttons.append(button);
    button->setParent(this);
    button->set_type("child");
    button->show();
}

void expanding_list_item::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    m_parent_button->repolish();
    m_parent_button->move(0, 0);
    m_collapsed_height = m_parent_button->height();
    int offset         = m_collapsed_height;

    for (expanding_list_button* button : m_child_buttons)
    {
        button->repolish();
        button->move(0, offset);
        offset += button->height();
    }

    m_expanded_height = offset;

    m_animation->setDuration(200);
    //    m_animation->setStartValue(QSize(width(), m_collapsed_height));
    //    m_animation->setEndValue(QSize(width(), m_expanded_height));

    m_animation->setStartValue(m_collapsed_height);
    m_animation->setEndValue(m_expanded_height);

    setFixedHeight(m_collapsed_height);
}

void expanding_list_item::collapse()
{
    for (expanding_list_button* button : m_child_buttons)
        button->setEnabled(false);

    m_animation->setDirection(QPropertyAnimation::Backward);

    if (m_animation->state() != QPropertyAnimation::Running)
        m_animation->start();
}

void expanding_list_item::expand()
{
    for (expanding_list_button* button : m_child_buttons)
        button->setEnabled(true);

    m_animation->setDirection(QPropertyAnimation::Forward);

    if (m_animation->state() == QPropertyAnimation::Running)
    {
        m_animation->pause();
        m_animation->resume();
    }
    else
        m_animation->start();
}

void expanding_list_item::set_expanded(bool expanded)
{
    m_expanded = expanded;
    repolish();
}

void expanding_list_item::set_fixed_height(int height)
{
    m_fixed_height = height;

    setMinimumHeight(height);
    setMaximumHeight(height);
}
}
