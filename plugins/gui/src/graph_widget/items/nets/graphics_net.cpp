#include "gui/graph_widget/items/nets/graphics_net.h"

#include "netlist/net.h"

#include <QPen>

namespace hal
{
    qreal GraphicsNet::s_line_width;
    qreal GraphicsNet::s_shape_width;

    QPen GraphicsNet::s_pen;
    QBrush GraphicsNet::s_brush;

    void GraphicsNet::load_settings()
    {
        s_line_width = 1.8;
        s_shape_width = 5;

        s_pen.setWidthF(s_line_width);
        s_pen.setJoinStyle(Qt::MiterJoin);
    }

    GraphicsNet::GraphicsNet(Net* n) : GraphicsItem(hal::item_type::net, n->get_id())
    {
    }

    QRectF GraphicsNet::boundingRect() const
    {
        return m_rect;
    }

    QPainterPath GraphicsNet::shape() const
    {
        return m_shape;
    }

    void GraphicsNet::set_visuals(const GraphicsNet::visuals& v)
    {
        setVisible(v.visible);

        m_color = v.color;
        m_pen_style = v.pen_style;
        m_fill_icon = v.fill_icon;
        m_fill_color = v.fill_color;
        m_brush_style = v.brush_style;
    }
}
