#include "graph_widget/graphics_items/standard_graphics_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

qreal standard_graphics_net::s_alpha;
qreal standard_graphics_net::s_radius; // STATIC CONST ?
QBrush standard_graphics_net::s_brush;

void standard_graphics_net::load_settings()
{
    s_radius = 3;
    s_brush.setStyle(Qt::SolidPattern);

    // TEST
    s_pen.setCosmetic(false);
    s_pen.setColor(QColor(160, 160, 160));
    // TEST END
}

void standard_graphics_net::update_alpha()
{
    if (s_lod >= graph_widget_constants::net_fade_in_lod && s_lod <= graph_widget_constants::net_fade_out_lod)
    {
        const qreal difference = graph_widget_constants::net_fade_out_lod - graph_widget_constants::net_fade_in_lod;

        s_alpha = (s_lod - graph_widget_constants::net_fade_in_lod) / difference;
    }
    else
        s_alpha = 1;
}

standard_graphics_net::standard_graphics_net(std::shared_ptr<net> n, const lines& l) : graphics_net(n)
{    
    QVector<h_line> collapsed_h;
    QVector<v_line> collapsed_v;

    for (const h_line& h : l.h_lines)
    {
        if (h.small_x == h.big_x)
            continue;

        QVector<int> overlaps;

        for (int i = 0; i < collapsed_h.size(); ++i)
            if (h.y == collapsed_h.at(i).y)
            {
                if (h.big_x < collapsed_h.at(i).small_x || collapsed_h.at(i).big_x < h.small_x)
                    continue; // NO OVERLAP

                // OVERLAP
                overlaps.append(i);
            }


        if (overlaps.isEmpty())
            collapsed_h.append(h);
        else
        {
            qreal smallest_x = h.small_x;
            qreal biggest_x = h.big_x;

            for (int i = 0; i < overlaps.size(); ++i)
            {
                int index = overlaps.at(i) - i;

                if (collapsed_h.at(index).small_x < smallest_x)
                    smallest_x = collapsed_h.at(index).small_x;

                if (collapsed_h.at(index).big_x > biggest_x)
                    biggest_x = collapsed_h.at(index).big_x;

                collapsed_h.remove(i);
            }

            collapsed_h.append(h_line{smallest_x, biggest_x, h.y});
        }
    }

    for (const v_line& v : l.v_lines)
    {
        if (v.small_y == v.big_y)
            continue;

        QVector<int> overlaps;

        for (int i = 0; i < collapsed_v.size(); ++i)
            if (v.x == collapsed_v.at(i).x)
            {
                if (v.big_y < collapsed_v.at(i).small_y || collapsed_v.at(i).big_y < v.small_y)
                    continue; // NO OVERLAP

                // OVERLAP
                overlaps.append(i);
            }


        if (overlaps.isEmpty())
            collapsed_v.append(v);
        else
        {
            qreal smallest_y = v.small_y;
            qreal biggest_y = v.big_y;

            for (int i = 0; i < overlaps.size(); ++i)
            {
                int index = overlaps.at(i) - i;

                if (collapsed_v.at(index).small_y < smallest_y)
                    smallest_y = collapsed_v.at(index).small_y;

                if (collapsed_v.at(index).big_y > biggest_y)
                    biggest_y = collapsed_v.at(index).big_y;

                collapsed_v.remove(i);
            }

            collapsed_v.append(v_line{v.x, smallest_y, biggest_y});
        }
    }

    // CALCULATE SPLITS
    for (const h_line& h : collapsed_h)
    {
        for (const v_line& v : collapsed_v)
        {
            if (h.small_x == v.x)
                if (h.y > v.small_y && h.y < v.big_y)
                    m_splits.append(QPointF(h.small_x - l.src_x, h.y - l.src_y));

            if (h.big_x == v.x)
                if (h.y > v.small_y && h.y < v.big_y)
                    m_splits.append(QPointF(h.big_x -l.src_x, h.y - l.src_y));
        }
    }

    // CALCULATE RECT AND SHAPE
    // 0 INITIALIZATION ONLY WORKS WHEN NETS ARE PLACED AT THE SRC POSITION, CHANGE ?
    qreal smallest_x = 0;
    qreal biggest_x = 0;
    qreal smallest_y = 0;
    qreal biggest_y = 0;

    for (const h_line& h : collapsed_h)
    {
        qreal small_x = h.small_x - l.src_x;
        qreal big_x = h.big_x - l.src_x;
        qreal y = h.y - l.src_y;

        if (small_x < smallest_x)
            smallest_x = small_x;

        if (big_x > biggest_x)
            biggest_x = big_x;

        if (y < smallest_y)
            smallest_y = y;
        else if (y > biggest_y)
            biggest_y = y;

        QLineF line(small_x, y, big_x, y);
        m_lines.append(line);
        QRectF rect(small_x - s_stroke_width / 2, y - s_stroke_width / 2, big_x - small_x + 1.5 + s_stroke_width, 1.5 + s_stroke_width); // use pen width variable / const
        m_shape.addRect(rect);
    }

    for (const v_line& v : collapsed_v)
    {
        qreal x = v.x - l.src_x;
        qreal small_y = v.small_y - l.src_y;
        qreal big_y = v.big_y - l.src_y;

        if (x < smallest_x)
            smallest_x = x;
        else if (x > biggest_x)
            biggest_x = x;

        if (small_y < smallest_y)
            smallest_y = small_y;

        if (big_y > biggest_y)
            biggest_y = big_y;

        QLineF line(x, small_y, x, big_y);
        m_lines.append(line);
        QRectF rect(x - s_stroke_width / 2, small_y - s_stroke_width / 2, big_y - small_y + 1.5 + s_stroke_width, 1.5 + s_stroke_width); // use pen width variable / const
        m_shape.addRect(rect);
    }

    m_rect = QRectF(smallest_x, smallest_y, biggest_x - smallest_x, biggest_y - smallest_y);
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
        QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
        color.setAlphaF(s_alpha);

        s_pen.setColor(color);
        s_brush.setColor(color);
        painter->setPen(s_pen);
        painter->setBrush(s_brush);

        bool antialiasing = painter->renderHints().testFlag(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::Antialiasing, true);

        for (const QPointF& point : m_splits)
            painter->drawEllipse(point, s_radius, s_radius);

        painter->setRenderHint(QPainter::Antialiasing, antialiasing);
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
