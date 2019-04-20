#include "graph_widget/graphics_items/graphics_net.h"
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

    s_stroke_width = 0.3;
}

graphics_net::graphics_net(std::shared_ptr<net> n)
{
    if (n)
        m_id = n->get_id();
    else
    {
        m_id = -1;
        log_error("gui graph", "inconsistency error (net is nullptr).");
    }
    m_class = item_class::net;
    m_net   = n;
}

std::shared_ptr<net> graphics_net::get_net()
{
    return m_net;
}

QRectF graphics_net::boundingRect() const
{
    return m_rect;
}

QPainterPath graphics_net::shape() const
{
    return m_shape;
}
