#include "gui/graph_widget/items/nets/hollow_arrow_separated_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal hollow_arrow_separated_net::s_wire_length;
qreal hollow_arrow_separated_net::s_input_arrow_offset;
qreal hollow_arrow_separated_net::s_output_arrow_offset;
qreal hollow_arrow_separated_net::s_arrow_length;
qreal hollow_arrow_separated_net::s_arrow_height;
qreal hollow_arrow_separated_net::s_arrow_left_outward_x_shift;
qreal hollow_arrow_separated_net::s_arrow_right_outward_x_shift;

qreal hollow_arrow_separated_net::s_input_offset;

qreal hollow_arrow_separated_net::s_input_width;
qreal hollow_arrow_separated_net::s_output_width;

QPainterPath hollow_arrow_separated_net::s_arrow;

QBrush hollow_arrow_separated_net::s_brush;

void hollow_arrow_separated_net::load_settings()
{
    s_wire_length = 26;
    s_input_arrow_offset = 3;
    s_output_arrow_offset = 3;
    s_arrow_length = 12;
    s_arrow_height = 6;
    s_arrow_left_outward_x_shift = 0;
    s_arrow_right_outward_x_shift = -3;

    s_input_offset = s_wire_length + s_input_arrow_offset + s_arrow_length;

    s_input_width = s_wire_length + s_input_arrow_offset + s_arrow_length + s_stroke_width * 1.5;
    s_output_width = s_wire_length + s_output_arrow_offset + s_arrow_length + s_stroke_width * 1.5;

    if (s_arrow_left_outward_x_shift > 0)
    {
        s_input_offset += s_arrow_left_outward_x_shift;

        s_input_width += s_arrow_left_outward_x_shift;
        s_output_width += s_arrow_left_outward_x_shift;
    }

    if (s_arrow_right_outward_x_shift < 0)
    {
        s_input_offset -= s_arrow_right_outward_x_shift;

        s_input_width -= s_arrow_right_outward_x_shift;
        s_output_width -= s_arrow_right_outward_x_shift;
    }

    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift, s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift + s_arrow_length, s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift + s_arrow_length - s_arrow_right_outward_x_shift, 0));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift + s_arrow_length, -s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift, -s_arrow_height / 2));
    s_arrow.closeSubpath();

    s_brush.setStyle(Qt::NoBrush);
    s_pen.setColor(QColor(160, 160, 160)); // USE STYLESHEETS
}

hollow_arrow_separated_net::hollow_arrow_separated_net(const std::shared_ptr<const net> n) : separated_graphics_net(n)
{

}

void hollow_arrow_separated_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(QPointF(s_wire_length + s_output_arrow_offset, 0));
        painter->drawPath(s_arrow);
        painter->restore();
    }

    for (const QPointF& position : m_input_wires)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(QPointF(position.x() - s_input_offset, position.y()));
        painter->drawPath(s_arrow);
        painter->restore();
    }

#ifdef HAL_DEBUG_GUI_GRAPHICS
    bool original_value = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    s_pen.setCosmetic(original_value);
#endif

    painter->setBrush(QBrush());
}

void hollow_arrow_separated_net::set_visuals(const graphics_net::visuals& v)
{
    setVisible(v.visible);
    m_color = v.color;
    m_line_style = v.style;
}

void hollow_arrow_separated_net::add_output()
{
    if (m_draw_output)
        return;

    m_draw_output = true;

    m_shape.moveTo(QPointF(-s_stroke_width, -s_stroke_width));
    m_shape.lineTo(QPointF(s_wire_length + s_stroke_width, -s_stroke_width));
    m_shape.lineTo(QPointF(s_wire_length + s_stroke_width, s_stroke_width));
    m_shape.lineTo(QPointF(-s_stroke_width, s_stroke_width));
    m_shape.closeSubpath();

    const qreal x = s_wire_length + s_output_arrow_offset - s_stroke_width;

    // SPACING EFFECTS NOT PROPERLY CALCULATED HERE
    m_shape.moveTo(QPointF(x, 0));
    m_shape.lineTo(QPointF(x + s_arrow_left_outward_x_shift, -s_arrow_height / 2 - s_stroke_width));
    m_shape.lineTo(QPointF(x + s_arrow_left_outward_x_shift + s_arrow_length + s_stroke_width, -s_arrow_height / 2 - s_stroke_width));
    m_shape.lineTo(QPointF(x + s_arrow_left_outward_x_shift + s_arrow_length - s_arrow_right_outward_x_shift + 2* s_stroke_width, 0));
    m_shape.lineTo(QPointF(x + s_arrow_left_outward_x_shift + s_arrow_length + s_stroke_width, s_arrow_height / 2 + s_stroke_width));
    m_shape.lineTo(QPointF(x + s_arrow_left_outward_x_shift, s_arrow_height / 2 + s_stroke_width));
    m_shape.closeSubpath();
}

void hollow_arrow_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_wires.append(mapped_position);

    qreal x = mapped_position.x() + s_stroke_width;
    const qreal y = mapped_position.y();

    m_shape.moveTo(QPointF(x, y - s_stroke_width));
    m_shape.lineTo(QPointF(x - s_wire_length - s_stroke_width * 2, y - s_stroke_width));
    m_shape.lineTo(QPointF(x - s_wire_length - s_stroke_width * 2, y + s_stroke_width));
    m_shape.lineTo(QPointF(x, y + s_stroke_width));
    m_shape.closeSubpath();

    // SPACING EFFECTS NOT PROPERLY CALCULATED HERE
    x -= s_wire_length + s_input_arrow_offset;

    m_shape.moveTo(QPointF(x, y));

    x += s_arrow_right_outward_x_shift;

    qreal outwards = s_arrow_height / 2 + s_stroke_width;

    m_shape.lineTo(QPointF(x, y - outwards));
    m_shape.lineTo(QPointF(x - s_arrow_length - 2 * s_stroke_width, y - outwards));
    m_shape.lineTo(QPointF(x - s_arrow_length - s_arrow_left_outward_x_shift - 2 * s_stroke_width, y));
    m_shape.lineTo(QPointF(x - s_arrow_length - 2 * s_stroke_width, y + outwards));
    m_shape.lineTo(QPointF(x, y + outwards));
    m_shape.closeSubpath();
}

void hollow_arrow_separated_net::finalize()
{
    qreal spacing = 1;

    if (s_line_width > spacing)
        spacing = s_line_width; // TOO BIG, DOESNT MATTER

    m_rect = m_shape.boundingRect();
    m_rect.adjust(-spacing, -spacing, spacing, spacing);
}

qreal hollow_arrow_separated_net::input_width() const
{  
    return s_input_width;
}

qreal hollow_arrow_separated_net::output_width() const
{
    return s_output_width;
}
