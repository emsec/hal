#include "window_manager/window_toolbar.h"

#include <QActionEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QStyle>
#include <QToolButton>

namespace hal
{
    WindowToolbar::WindowToolbar(QWidget* parent) : QFrame(parent),
        m_layout(new QHBoxLayout(this))
    {
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(2); // USE PROPERTY
    }

    void WindowToolbar::add_widget(QWidget* widget)
    {
        m_layout->addWidget(widget);
    }

    void WindowToolbar::add_spacer()
    {
        QWidget* spacer = new QWidget(this);
        spacer->setAttribute(Qt::WA_NoSystemBackground);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_layout->addWidget(spacer);
    }

    void WindowToolbar::clear()
    {
        QLayoutItem* item = nullptr;
        while ((item = m_layout->takeAt(0)))
        {
            if (item->layout())
                item->layout()->deleteLater();

            if (item->widget())
                item->widget()->deleteLater();

            delete item;
        }
    }

    void WindowToolbar::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    void WindowToolbar::actionEvent(QActionEvent* event)
    {
        switch (event->type())
        {
        case QEvent::ActionAdded:
        {
            QToolButton* button = new QToolButton();
            button->setAutoRaise(true);
            button->setFocusPolicy(Qt::NoFocus);
            button->setIconSize(QSize(18, 18)); // USE PROPERTY
            button->setMinimumSize(QSize(18, 18));
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
            button->setDefaultAction(event->action());
            button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            add_widget(button);
            break;
        }
        default:
        {
            QWidget::actionEvent(event);
            break;
        }
        }
    }
}
