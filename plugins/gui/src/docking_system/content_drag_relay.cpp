#include "gui/docking_system/content_drag_relay.h"

namespace hal
{
    ContentDragRelay *ContentDragRelay::instance()
    {
        static ContentDragRelay instance;
        return &instance;
    }

    void ContentDragRelay::relayDragStart()
    {
        Q_EMIT dragStart();
    }

    void ContentDragRelay::relayDragEnd()
    {
        Q_EMIT dragEnd();
    }

    ContentDragRelay::ContentDragRelay(QObject *parent) : QObject(parent)
    {

    }
}
