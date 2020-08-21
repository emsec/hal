#include "gui/expanding_list/expanding_list_item.h"

#include "gui/expanding_list/expanding_list_button.h"

#include <QPropertyAnimation>
#include <QStyle>

namespace hal
{
    ExpandingListItem::ExpandingListItem(ExpandingListButton* parent_button, QWidget* parent)
        : QFrame(parent), m_parent_button(parent_button), m_collapsed_height(0), m_expanded_height(0),
          //m_animation(new QPropertyAnimation(this, "minimumHeight", this)),
          m_animation(new QPropertyAnimation(this, "fixed_height", this)), m_expanded(false), m_fixed_height(m_parent_button->minimumHeight())
    {
        parent_button->setParent(this);
        parent_button->show();
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMinimumHeight(m_parent_button->minimumHeight());
    }

    QSize ExpandingListItem::minimumSizeHint() const
    {
        return minimumSize();
    }

    QSize ExpandingListItem::sizeHint() const
    {
        return size();
    }

    void ExpandingListItem::resizeEvent(QResizeEvent* event)
    {
        int width = event->size().width();

        m_parent_button->resize(width, m_parent_button->height());

        for (ExpandingListButton* button : m_child_buttons)
            button->resize(width, button->height());
    }

    bool ExpandingListItem::expanded()
    {
        return m_expanded;
    }

    int ExpandingListItem::fixed_height()
    {
        return m_fixed_height;
    }

    bool ExpandingListItem::contains(ExpandingListButton* button)
    {
        if (button == m_parent_button)
            return true;

        for (ExpandingListButton* child_button : m_child_buttons)
            if (button == child_button)
                return true;

        return false;
    }

    ExpandingListButton* ExpandingListItem::parent_button()
    {
        return m_parent_button;
    }

    void ExpandingListItem::append_child_button(ExpandingListButton* button)
    {
        m_child_buttons.append(button);
        button->setParent(this);
        button->set_type("child");
        button->show();
    }

    void ExpandingListItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        m_parent_button->repolish();
        m_parent_button->move(0, 0);
        m_collapsed_height = m_parent_button->height();
        int offset         = m_collapsed_height;

        for (ExpandingListButton* button : m_child_buttons)
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

    void ExpandingListItem::collapse()
    {
        for (ExpandingListButton* button : m_child_buttons)
            button->setEnabled(false);

        m_animation->setDirection(QPropertyAnimation::Backward);

        if (m_animation->state() != QPropertyAnimation::Running)
            m_animation->start();
    }

    void ExpandingListItem::expand()
    {
        for (ExpandingListButton* button : m_child_buttons)
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

    void ExpandingListItem::set_expanded(bool expanded)
    {
        m_expanded = expanded;
        repolish();
    }

    void ExpandingListItem::set_fixed_height(int height)
    {
        m_fixed_height = height;

        setMinimumHeight(height);
        setMaximumHeight(height);
    }
}
