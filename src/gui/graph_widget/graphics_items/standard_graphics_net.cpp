#include "graph_widget/graphics_items/standard_graphics_net.h"
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

standard_graphics_net::standard_graphics_net(std::shared_ptr<net> n) : graphics_net(n)
{
}

void standard_graphics_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    s_pen.setColor((option->state & QStyle::State_Selected) ? s_selection_color : m_color);
    painter->setPen(s_pen);

    painter->drawLines(m_lines);
}

void standard_graphics_net::finalize()
{
    QPainterPathStroker stroker;
    stroker.setWidth(s_stroke_width);
    m_rect  = m_path.boundingRect();
    m_shape = stroker.createStroke(m_path);
}

void standard_graphics_net::line_to(const QPointF& scene_position)
{
    if (scene_position == current_scene_position())
        return;

    QPointF mapped_point = mapFromScene(scene_position);
    m_lines.append(QLineF(m_path.currentPosition(), mapped_point));
    m_path.lineTo(mapped_point);
}

void standard_graphics_net::move_pen_to(const QPointF& scene_position)
{
    if (scene_position == current_scene_position())
        return;

    QPointF mapped_point = mapFromScene(scene_position);
    m_path.moveTo(mapped_point);
}

QPointF standard_graphics_net::current_scene_position() const
{
    return mapToScene(m_path.currentPosition());
}
