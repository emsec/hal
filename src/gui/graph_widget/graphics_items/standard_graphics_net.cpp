#include "graph_widget/graphics_items/standard_graphics_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

qreal standard_graphics_net::s_alpha;
qreal standard_graphics_net::s_radius; // STATIC CONST ?
QBrush standard_graphics_net::s_brush;

standard_graphics_net::standard_graphics_net(std::shared_ptr<net> n, const lines& l) : graphics_net(n)
{
    QVector<h_line> collapsed_h;
    QVector<v_line> collapsed_v;

    for (const h_line& h : l.h_lines)
    {
        assert(h.x1 != h.x2);

        bool match = false;

        for (int i = 0; i < collapsed_h.size(); ++i)
            if (h.y == collapsed_h.at(i).y)
            {
                qreal new_line_small_x = h.x1;
                qreal new_line_big_x = h.x2;

                if (h.x1 > h.x2)
                {
                    new_line_small_x = h.x2;
                    new_line_big_x = h.x1;
                }

                qreal old_line_small_x = collapsed_h.at(i).x1;
                qreal old_line_big_x = collapsed_h.at(i).x2;

                if (h.x1 > h.x2)
                {
                    old_line_small_x = collapsed_h.at(i).x2;
                    old_line_big_x = collapsed_h.at(i).x1;
                }

                if (new_line_big_x < old_line_small_x || old_line_big_x < new_line_small_x)
                    continue; // NO OVERLAP

                // OVERLAP
                // SAVE INDEX
                qreal smallest_x = new_line_small_x;

                if (old_line_small_x < new_line_small_x)
                    smallest_x = old_line_small_x;

                qreal biggest_x = new_line_big_x;

                if (old_line_big_x > new_line_big_x)
                    biggest_x = old_line_big_x;

                match = true;
                break;
            }


        if (!match)
            collapsed_h.append(h);
    }

}

void standard_graphics_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    s_pen.setColor((option->state & QStyle::State_Selected) ? s_selection_color : m_color);
    painter->setPen(s_pen);

    //    if (g_selection_relay.m_focus_type == selection_relay::item_type::net)
    //        if (g_selection_relay.m_focus_net_id == m_id)
    //        {
    //            QPen pen(s_selection_color, 1, Qt::DashLine);
    //            pen.setJoinStyle(Qt::MiterJoin);
    //            //pen.setCosmetic(true);
    //            painter->setPen(pen);
    //        }

    painter->drawLines(m_lines);

    if (s_lod > graph_widget_constants::net_fade_in_lod)
    {
        for (const QPointF& point : m_splits)
            painter->drawEllipse(point, s_radius, s_radius);
    }

#ifdef HAL_DEBUG_GUI_GRAPHICS
    s_pen.setColor(Qt::green);
    const bool cosmetic = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    s_pen.setCosmetic(cosmetic);
#endif
}

void standard_graphics_net::finalize()
{
    QPainterPathStroker stroker;
    stroker.setWidth(s_stroke_width);
    m_rect  = m_path.boundingRect();
    m_shape = stroker.createStroke(m_path);
}

void standard_graphics_net::line_to_x(const qreal scene_x)
{
    if (scene_x == current_scene_position().x())
        return;

    QPointF mapped_point = mapFromScene(scene_x, current_scene_position().y());
    m_lines.append(QLineF(m_path.currentPosition(), mapped_point));
    m_path.lineTo(mapped_point);
}

void standard_graphics_net::line_to_y(const qreal scene_y)
{
    if (scene_y == current_scene_position().y())
        return;

    QPointF mapped_point = mapFromScene(current_scene_position().x(), scene_y);
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
