#include "gui/notifications/notification_manager_qss_adapter.h"

#include <QStyle>

namespace hal
{
    NotificationManagerQssAdapter::NotificationManagerQssAdapter(QWidget* parent) : QWidget(parent)
    {
        //DEFAULT VALUES
        mXOffset = 20;
        mYOffset = 20;
        mSpacing  = 10;

        repolish();
    }

    void NotificationManagerQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    int NotificationManagerQssAdapter::xOffset() const
    {
        return mXOffset;
    }

    int NotificationManagerQssAdapter::yOffset() const
    {
        return mYOffset;
    }

    int NotificationManagerQssAdapter::spacing() const
    {
        return mSpacing;
    }

    void NotificationManagerQssAdapter::setXOffset(const int offset)
    {
        mXOffset = offset;
    }

    void NotificationManagerQssAdapter::setYOffset(const int offset)
    {
        mYOffset = offset;
    }

    void NotificationManagerQssAdapter::setSpacing(const int spacing)
    {
        mSpacing = spacing;
    }
}
