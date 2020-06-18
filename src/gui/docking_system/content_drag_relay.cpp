#include "docking_system/content_drag_relay.h"

namespace hal{
content_drag_relay *content_drag_relay::instance()
{
    static content_drag_relay instance;
    return &instance;
}

void content_drag_relay::relay_drag_start()
{
    Q_EMIT drag_start();
}

void content_drag_relay::relay_drag_end()
{
    Q_EMIT drag_end();
}

content_drag_relay::content_drag_relay(QObject *parent) : QObject(parent)
{

}
}
