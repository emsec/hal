#include "gui/notifications/notification_manager.h"
#include "gui/notifications/notification.h"

#include <QApplication>
#include <QDesktopWidget>

namespace hal
{
    NotificationManager::NotificationManager(QObject* parent) : QObject(parent)
    {
        mWidthOffset  = 20;
        mHeightOffset = 20;

        mSpacing = 10;
    }

    void NotificationManager::remove(Notification* n)
    {
        if (mList.removeOne(n))
        {
            n->hide();
            n->deleteLater();
        }
        rearrangeNotifications();
    }

    void NotificationManager::debugAddNotification()
    {
        Notification* n = new Notification(nullptr);
        mList.append(n);
        rearrangeNotifications();
    }

    void NotificationManager::rearrangeNotifications()
    {
        //QRect rec = QApplication::desktop()->availableGeometry();
        QRect rec        = QApplication::desktop()->screenGeometry();
        mDesktopWidth  = rec.width();
        mDesktopHeight = rec.height();

        int y = mHeightOffset;

        for (auto& element : mList)
        {
            y += element->height();
            element->move(mDesktopWidth - mWidthOffset - element->width(), mDesktopHeight - y);
            y += mSpacing;
            element->show();
            element->fadeIn();
        }
    }
}
