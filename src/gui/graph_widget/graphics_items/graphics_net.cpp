#include "gui/graph_widget/graphics_items/graphics_net.h"

#include "core/log.h"

#include "netlist/net.h"

#include <QPen>

QPen graphics_net::s_pen;
qreal graphics_net::s_stroke_width;

void graphics_net::load_settings()
{
    s_pen.setCosmetic(true);
    s_pen.setWidthF(1.5);
    s_pen.setJoinStyle(Qt::MiterJoin);

    s_stroke_width = 1.5;
}

graphics_net::graphics_net(std::shared_ptr<net> n) : graphics_item(item_type::net, n->get_id()),
    m_net(n)
{
    assert(n);
}

QRectF graphics_net::boundingRect() const
{
    return m_rect;
}

QPainterPath graphics_net::shape() const
{
    return m_shape;
}

std::shared_ptr<net> graphics_net::get_net()
{
    return m_net;
}
