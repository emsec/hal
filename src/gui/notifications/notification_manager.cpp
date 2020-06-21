#include "notifications/notification_manager.h"
#include "notifications/notification.h"

#include "QApplication"
#include "QDesktopWidget"

namespace hal
{
    NotificationManager::NotificationManager(QObject* parent) : QObject(parent)
    {
        m_width_offset  = 20;
        m_height_offset = 20;

        m_spacing = 10;
    }

    void NotificationManager::remove(Notification* n)
    {
        if (m_list.removeOne(n))
        {
            n->hide();
            n->deleteLater();
        }
        rearrange_Notifications();
    }

    void NotificationManager::debug_add_Notification()
    {
        Notification* n = new Notification(nullptr);
        m_list.append(n);
        rearrange_Notifications();
    }

    void NotificationManager::rearrange_Notifications()
    {
        //QRect rec = QApplication::desktop()->availableGeometry();
        QRect rec        = QApplication::desktop()->screenGeometry();
        m_desktop_width  = rec.width();
        m_desktop_height = rec.height();

        int y = m_height_offset;

        for (auto& element : m_list)
        {
            y += element->height();
            element->move(m_desktop_width - m_width_offset - element->width(), m_desktop_height - y);
            y += m_spacing;
            element->show();
            element->fade_in();
        }
    }
}
