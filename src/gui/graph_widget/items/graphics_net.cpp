#include "gui/graph_widget/items/graphics_net.h"

#include "core/log.h"

#include "netlist/net.h"

#include <QPen>

QPen graphics_net::s_pen;

qreal graphics_net::s_line_width;
qreal graphics_net::s_stroke_width;

void graphics_net::load_settings()
{
    s_line_width = 1.5; // USE CONSTANT ?
    s_pen.setWidthF(s_line_width);
    s_pen.setJoinStyle(Qt::MiterJoin);

    s_stroke_width = s_line_width;
}

graphics_net::graphics_net(const std::shared_ptr<const net> n) : graphics_item(hal::item_type::net, n->get_id())
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
