#include "graph_widget/items/nets/circle_separated_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <assert.h>

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal circle_separated_net::s_wire_length;
qreal circle_separated_net::s_circle_offset;
qreal circle_separated_net::s_radius;

QBrush circle_separated_net::s_brush;

void circle_separated_net::load_settings()
{
    s_wire_length   = 26;
    s_circle_offset = 0;
    s_radius        = 3;

    s_brush.setStyle(Qt::SolidPattern);
    s_pen.setColor(QColor(160, 160, 160)); // USE STYLESHEETS
}

circle_separated_net::circle_separated_net(const std::shared_ptr<const net> n) : separated_graphics_net(n)
{

}

void circle_separated_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    if (s_lod < graph_widget_constants::separated_net_min_lod)
        return;

    QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
    color.setAlphaF(s_alpha);

    s_pen.setColor(color);
    s_brush.setColor(color);
    painter->setPen(s_pen);
    painter->setBrush(s_brush);

    if (m_draw_output)
    {
        painter->drawLine(QPointF(0, 0), QPointF(s_wire_length, 0));
        bool original_antialiasing = painter->renderHints() & QPainter::Antialiasing;
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawEllipse(QPointF(s_wire_length + s_circle_offset, 0), s_radius, s_radius);
        painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    }

    bool original_antialiasing = painter->renderHints() & QPainter::Antialiasing;

    for (const QPointF& position : m_input_wires)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawEllipse(QPointF(to.x() - s_circle_offset, to.y()), s_radius, s_radius);
        painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    }

#ifdef HAL_DEBUG_GUI_GRAPHICS
    bool original_cosmetic = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    painter->setRenderHint(QPainter::Antialiasing, true);
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    s_pen.setCosmetic(original_cosmetic);
#endif

    painter->setBrush(QBrush());
}

void circle_separated_net::set_visuals(const graphics_net::visuals& v)
{
    setVisible(v.visible);
    m_color = v.color;
    m_line_style = v.style;
}

void circle_separated_net::add_output()
{
    assert(!m_draw_output);

    m_draw_output = true;

    m_shape.moveTo(QPointF(-s_stroke_width, -s_stroke_width));
    m_shape.lineTo(QPointF(s_wire_length + s_stroke_width, -s_stroke_width));
    m_shape.lineTo(QPointF(s_wire_length + s_stroke_width, s_stroke_width));
    m_shape.lineTo(QPointF(-s_stroke_width, s_stroke_width));
    m_shape.closeSubpath();

    const QPointF point(s_wire_length + s_circle_offset, 0);
    const qreal radius = s_radius + s_stroke_width;

    m_shape.addEllipse(point, radius, radius);

    // MAYBE BOUNDING RECT IS BETTER HERE
}

void circle_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_wires.append(mapped_position);

    const qreal x = mapped_position.x() + s_stroke_width;
    const qreal y = mapped_position.y();

    m_shape.moveTo(QPointF(x, y - s_stroke_width));
    m_shape.lineTo(QPointF(x - s_wire_length - s_stroke_width * 2, y - s_stroke_width));
    m_shape.lineTo(QPointF(x - s_wire_length - s_stroke_width * 2, y + s_stroke_width));
    m_shape.lineTo(QPointF(x, y + s_stroke_width));
    m_shape.closeSubpath();

    const QPointF point(mapped_position.x() - s_wire_length - s_circle_offset, mapped_position.y());
    const qreal radius = s_radius + s_stroke_width;

    m_shape.addEllipse(point, radius, radius);

    // MAYBE BOUNDING RECT IS BETTER HERE
}

void circle_separated_net::finalize()
{
    qreal spacing = 1;

    if (s_line_width > spacing)
        spacing = s_line_width; // TOO BIG, DOESNT MATTER

    m_rect = m_shape.boundingRect();
    m_rect.adjust(-spacing, -spacing, spacing, spacing);
}

qreal circle_separated_net::input_width() const
{
    return s_wire_length + s_circle_offset + 2 * s_radius;
}

qreal circle_separated_net::output_width() const
{
    return s_wire_length + s_circle_offset + 2 * s_radius;
}
