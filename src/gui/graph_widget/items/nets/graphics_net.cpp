#include "gui/graph_widget/items/nets/graphics_net.h"

#include "netlist/net.h"

#include <QPen>

namespace hal
{
    qreal graphics_net::s_line_width;
    qreal graphics_net::s_shape_width;

    QPen graphics_net::s_pen;
    QBrush graphics_net::s_brush;

    void graphics_net::load_settings()
    {
        s_line_width = 1.8;
        s_shape_width = 5;

        s_pen.setWidthF(s_line_width);
        s_pen.setJoinStyle(Qt::MiterJoin);
    }

    graphics_net::graphics_net(const std::shared_ptr<const Net> n) : graphics_item(hal::item_type::net, n->get_id())
    {
    }

    QRectF graphics_net::boundingRect() const
    {
        return m_rect;
    }

    QPainterPath graphics_net::shape() const
    {
        return m_shape;
    }

    void graphics_net::set_visuals(const graphics_net::visuals& v)
    {
        setVisible(v.visible);

        m_color = v.color;
        m_pen_style = v.pen_style;
        m_fill_icon = v.fill_icon;
        m_fill_color = v.fill_color;
        m_brush_style = v.brush_style;
    }
}
