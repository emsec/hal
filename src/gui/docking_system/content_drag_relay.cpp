#include "docking_system/content_drag_relay.h"

namespace hal
{
    ContentDragRelay *ContentDragRelay::instance()
    {
        static ContentDragRelay instance;
        return &instance;
    }

    void ContentDragRelay::relay_drag_start()
    {
        Q_EMIT drag_start();
    }

    void ContentDragRelay::relay_drag_end()
    {
        Q_EMIT drag_end();
    }

    ContentDragRelay::ContentDragRelay(QObject *parent) : QObject(parent)
    {

    }
}
