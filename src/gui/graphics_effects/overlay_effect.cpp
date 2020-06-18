#include "graphics_effects/overlay_effect.h"

#include <QPainter>
namespace hal{
overlay_effect::overlay_effect(QObject* parent) : QGraphicsEffect(parent)
{
}

void overlay_effect::draw(QPainter* painter)
{
    QPixmap pixmap;
    QPoint offset;

    if (sourceIsPixmap())
        pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
    else
    {
        pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
        //painter->setWorldTransform(QTransform());
    }

    painter->drawPixmap(offset, pixmap);
    painter->setBrush(QColor(0, 0, 0));
    painter->setOpacity(0.5);
    painter->drawRect(pixmap.rect());
}
}
