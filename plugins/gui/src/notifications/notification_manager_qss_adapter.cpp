#include "gui/notifications/notification_manager_qss_adapter.h"

#include <QStyle>

namespace hal
{
    NotificationManagerQssAdapter::NotificationManagerQssAdapter(QWidget* parent) : QWidget(parent)
    {
        //DEFAULT VALUES
        m_x_offset = 20;
        m_y_offset = 20;
        m_spacing  = 10;

        repolish();
    }

    void NotificationManagerQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    int NotificationManagerQssAdapter::x_offset() const
    {
        return m_x_offset;
    }

    int NotificationManagerQssAdapter::y_offset() const
    {
        return m_y_offset;
    }

    int NotificationManagerQssAdapter::spacing() const
    {
        return m_spacing;
    }

    void NotificationManagerQssAdapter::set_x_offset(const int offset)
    {
        m_x_offset = offset;
    }

    void NotificationManagerQssAdapter::set_y_offset(const int offset)
    {
        m_y_offset = offset;
    }

    void NotificationManagerQssAdapter::set_spacing(const int spacing)
    {
        m_spacing = spacing;
    }
}
